// SPDX-License-Identifier: GPL-3.0-or-later
#include "ExternalGameRuntime.h"
#include "PluginDiagnostics.h"
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
};
ExternalGameRuntime::ExternalGameRuntime(PluginDiagnostics*d,QObject*p):QObject(p),m_impl(std::make_unique<Impl>()),m_diagnostics(d){}
ExternalGameRuntime::~ExternalGameRuntime(){unload();}
QObject *ExternalGameRuntime::item()const{return m_item.data();} QString ExternalGameRuntime::status()const{return m_status;} QString ExternalGameRuntime::lastError()const{return m_error;}
void ExternalGameRuntime::setServices(const QHash<QString,QObject*>&services){m_services=services;}
void ExternalGameRuntime::unload()
{
    if (m_item) {
        m_item->setParentItem(nullptr);
        delete m_item.data();
        m_item.clear();
    }
    if (m_impl->engine) {
        delete m_impl->engine;
        m_impl->engine = nullptr;
    }
    // QQmlEngine does not own QQmlNetworkAccessManagerFactory. Keep it alive
    // until the engine is destroyed, then release it explicitly.
    m_impl->networkFactory.reset();
    m_status = QStringLiteral("null");
    m_error.clear();
    emit changed();
}
bool ExternalGameRuntime::load(QQuickItem *parentItem,const QUrl &source,const QString &gameId,bool allowHttpsNetwork)
{
    unload(); if(!parentItem||!source.isValid()||source.isEmpty()){m_status=QStringLiteral("error");m_error=QStringLiteral("Invalid external game parent or source URL.");emit changed();emit failed(m_error);return false;}
    const QString scheme=source.scheme().toLower();if(scheme!=QStringLiteral("qrc")){m_status=QStringLiteral("error");m_error=QStringLiteral("External games must load from validated qrc:/ resources.");emit changed();emit failed(m_error);return false;}
    m_status=QStringLiteral("loading");emit changed();
    m_impl->engine = new QQmlEngine(this);
    m_impl->networkFactory = std::make_unique<RestrictedNamFactory>(allowHttpsNetwork);
    m_impl->engine->setNetworkAccessManagerFactory(m_impl->networkFactory.get());
    if (m_diagnostics)
        m_diagnostics->attachEngine(m_impl->engine);
    QQmlContext *context=new QQmlContext(m_impl->engine->rootContext(),m_impl->engine);for(auto it=m_services.cbegin();it!=m_services.cend();++it)context->setContextProperty(it.key(),it.value());context->setContextProperty(QStringLiteral("ExternalGameId"),gameId);
    QQmlComponent component(m_impl->engine,source,QQmlComponent::PreferSynchronous);if(component.isError()){QStringList errors;for(const QQmlError&e:component.errors())errors<<e.toString();m_error=errors.join(QLatin1Char('\n'));m_status=QStringLiteral("error");if(m_diagnostics)m_diagnostics->log(QStringLiteral("error"),m_error,source.toString(),0);delete m_impl->engine; m_impl->engine=nullptr; m_impl->networkFactory.reset(); emit changed(); emit failed(m_error); return false;}
    QObject *object=component.create(context);QQuickItem *quick=qobject_cast<QQuickItem*>(object);if(!quick){if(object)object->deleteLater();m_error=QStringLiteral("External game root must be a QQuickItem.");m_status=QStringLiteral("error");delete m_impl->engine; m_impl->engine=nullptr; m_impl->networkFactory.reset(); emit changed(); emit failed(m_error); return false;}
    QQmlEngine::setObjectOwnership(quick,QQmlEngine::CppOwnership);quick->setParent(this);quick->setParentItem(parentItem);quick->setWidth(parentItem->width());quick->setHeight(parentItem->height());connect(parentItem,&QQuickItem::widthChanged,quick,[parentItem,quick]{quick->setWidth(parentItem->width());});connect(parentItem,&QQuickItem::heightChanged,quick,[parentItem,quick]{quick->setHeight(parentItem->height());});
    m_item=quick;m_status=QStringLiteral("ready");m_error.clear();emit changed();emit loaded(quick);return true;
}
