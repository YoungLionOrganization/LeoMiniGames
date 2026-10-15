// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "ExternalGameRuntime.h"
#include "PluginDiagnostics.h"
#include "GameSave.h"
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QNetworkAccessManager>
#include <QQmlNetworkAccessManagerFactory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

class DeniedNetworkReply final : public QNetworkReply
{
public:
    DeniedNetworkReply(const QNetworkRequest &request, QObject *parent)
        : QNetworkReply(parent)
    {
        setRequest(request);
        setUrl(request.url());
        setError(QNetworkReply::ContentAccessDenied,
                 QStringLiteral("External game network access denied by host policy."));
        open(QIODevice::ReadOnly | QIODevice::Unbuffered);
        QTimer::singleShot(0, this, [this] {
            setFinished(true);
            emit errorOccurred(error());
            emit finished();
        });
    }
    void abort() override {} qint64 readData(char*,qint64) override{return -1;}
};
class RestrictedNam final : public QNetworkAccessManager
{
public:
    RestrictedNam(bool allowHttps,QObject *parent=nullptr):QNetworkAccessManager(parent),m_allowHttps(allowHttps){}
protected:
    QNetworkReply *createRequest(Operation op,const QNetworkRequest &request,QIODevice *outgoing=nullptr) override
    {
        const QString scheme=request.url().scheme().toLower();
        if(m_allowHttps && scheme==QStringLiteral("https")) return QNetworkAccessManager::createRequest(op,request,outgoing);
        return new DeniedNetworkReply(request,this);
    }
private: bool m_allowHttps=false;
};
class RestrictedNamFactory final : public QQmlNetworkAccessManagerFactory
{
public: explicit RestrictedNamFactory(bool allowHttps):m_allowHttps(allowHttps){} QNetworkAccessManager *create(QObject *parent) override{return new RestrictedNam(m_allowHttps,parent);} private: bool m_allowHttps=false;
};

struct ExternalGameRuntime::Impl
{
    QQmlEngine *engine = nullptr;
    std::unique_ptr<RestrictedNamFactory> networkFactory;
    quint64 generation = 0;
    bool destroying = false;
};
ExternalGameRuntime::ExternalGameRuntime(PluginDiagnostics*d,QObject*p):QObject(p),m_impl(std::make_unique<Impl>()),m_diagnostics(d){}
ExternalGameRuntime::~ExternalGameRuntime(){
    m_impl->destroying=true;unload();
    qDeleteAll(findChildren<QQuickItem *>(QString(),Qt::FindDirectChildrenOnly));
    qDeleteAll(findChildren<QQmlEngine *>(QString(),Qt::FindDirectChildrenOnly));
}
QObject *ExternalGameRuntime::item()const{return m_item.data();} QString ExternalGameRuntime::status()const{return m_status;} QString ExternalGameRuntime::lastError()const{return m_error;}
void ExternalGameRuntime::setServices(const QHash<QString,QObject*>&services){m_services=services;}
void ExternalGameRuntime::unload()
{
    const quint64 generation=++m_impl->generation;
    QPointer<QQuickItem> oldItem = m_item;
    QPointer<QQmlEngine> oldEngine = m_impl->engine;
    const auto oldFactory = std::shared_ptr<RestrictedNamFactory>(m_impl->networkFactory.release());
    m_item.clear();
    m_impl->engine = nullptr;
    m_status = QStringLiteral("null");
    m_error.clear();
    if (oldEngine) {
        if (auto *save = qobject_cast<GameSave *>(m_services.value(QStringLiteral("GameSave")))) save->setEngine(nullptr);
    }
    if (oldItem) { oldItem->setVisible(false); oldItem->setParentItem(nullptr); }
    // A legacy close call can arrive from inside the engine being closed.
    // Retire its objects after that JavaScript invocation returns.
    QTimer::singleShot(0, this, [oldItem, oldEngine, oldFactory] {
        delete oldItem.data();
        delete oldEngine.data();
        // Captured factory survives until the retired engine has been destroyed.
    });
    if(generation==m_impl->generation&&!m_impl->destroying)emit changed();
}
bool ExternalGameRuntime::load(QQuickItem *parentItem,const QUrl &source,const QString &gameId,bool allowHttpsNetwork)
{
    if(m_impl->destroying)return false;
    QPointer<QQuickItem> parent=parentItem;
    const quint64 generation=m_impl->generation+1;
    unload();
    if(generation!=m_impl->generation)return false;
    const auto fail=[this,generation,&source](const QString &message) {
        if(generation!=m_impl->generation)return false;
        const quint64 retired=generation+1;
        unload();
        if(retired!=m_impl->generation)return false;
        m_status=QStringLiteral("error");m_error=message;
        if(m_diagnostics)m_diagnostics->log(QStringLiteral("error"),message,source.toString(),0);
        if(retired!=m_impl->generation)return false;
        emit changed();
        if(retired==m_impl->generation)emit failed(message);
        return false;
    };
    if(!parent||!source.isValid()||source.isEmpty())return fail(QStringLiteral("Invalid external game parent or source URL."));
    if(source.scheme().compare(QStringLiteral("qrc"),Qt::CaseInsensitive)!=0)
        return fail(QStringLiteral("External games must load from validated qrc:/ resources."));
    m_status=QStringLiteral("loading");emit changed();
    if(generation!=m_impl->generation)return false;
    if(!parent)return fail(QStringLiteral("External game parent was destroyed during loading."));
    m_impl->engine = new QQmlEngine(this);
    if (auto *save = qobject_cast<GameSave *>(m_services.value(QStringLiteral("GameSave")))) save->setEngine(m_impl->engine);
    m_impl->networkFactory = std::make_unique<RestrictedNamFactory>(allowHttpsNetwork);
    m_impl->engine->setNetworkAccessManagerFactory(m_impl->networkFactory.get());
    if (m_diagnostics)
        m_diagnostics->attachEngine(m_impl->engine);
    QQmlContext *context=new QQmlContext(m_impl->engine->rootContext(),m_impl->engine);for(auto it=m_services.cbegin();it!=m_services.cend();++it)context->setContextProperty(it.key(),it.value());context->setContextProperty(QStringLiteral("ExternalGameId"),gameId);
    QQmlComponent component(m_impl->engine,source,QQmlComponent::PreferSynchronous);
    const auto componentError=[&component] {
        QStringList errors;for(const QQmlError &error:component.errors())errors<<error.toString();
        return errors.isEmpty()?QStringLiteral("External game component is not ready."):errors.join(QLatin1Char('\n'));
    };
    if(generation!=m_impl->generation)return false;
    if(!parent)return fail(QStringLiteral("External game parent was destroyed during loading."));
    if(!component.isReady())return fail(componentError());
    QPointer<QObject> object=component.beginCreate(context);
    if(generation!=m_impl->generation) {
        component.completeCreate();if(object)object->deleteLater();return false;
    }
    if(!parent) {
        component.completeCreate();delete object.data();
        return fail(QStringLiteral("External game parent was destroyed during loading."));
    }
    QQuickItem *quick=qobject_cast<QQuickItem*>(object.data());
    if(!quick) {
        component.completeCreate();delete object.data();
        return fail(component.isError()?componentError():QStringLiteral("External game root must be a QQuickItem."));
    }
    const QSizeF parentSize(parent->width(),parent->height());
    QQmlEngine::setObjectOwnership(quick,QQmlEngine::CppOwnership);quick->setParent(this);
    m_item=quick;
    connect(parent,&QQuickItem::widthChanged,quick,[this,generation,parent,quick]{if(parent&&generation==m_impl->generation&&m_item==quick)quick->setWidth(parent->width());});
    connect(parent,&QQuickItem::heightChanged,quick,[this,generation,parent,quick]{if(parent&&generation==m_impl->generation&&m_item==quick)quick->setHeight(parent->height());});
    quick->setParentItem(parent);
    if(generation==m_impl->generation)quick->setWidth(parentSize.width());
    if(generation==m_impl->generation)quick->setHeight(parentSize.height());
    component.completeCreate();
    if(generation!=m_impl->generation)return false;
    if(!parent)return fail(QStringLiteral("External game parent was destroyed during loading."));
    if(component.isError())return fail(componentError());
    if(m_item!=quick)return false;
    m_status=QStringLiteral("ready");m_error.clear();emit changed();
    if(generation!=m_impl->generation||m_item!=quick)return false;
    emit loaded(quick);
    return generation==m_impl->generation&&m_item==quick;
}
