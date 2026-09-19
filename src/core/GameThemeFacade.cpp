// SPDX-License-Identifier: LicenseRef-LMG-SAPEL-1.0
#include "GameThemeFacade.h"
#include "ThemeManager.h"
GameThemeFacade::GameThemeFacade(ThemeManager *theme,QObject *parent):QObject(parent),m_theme(theme)
{
    if(!m_theme) return;
    connect(m_theme,&ThemeManager::activeThemeChanged,this,&GameThemeFacade::activeThemeChanged);
    connect(m_theme,&ThemeManager::revisionChanged,this,&GameThemeFacade::revisionChanged);
    connect(m_theme,&ThemeManager::themesChanged,this,&GameThemeFacade::themesChanged);
}
QString GameThemeFacade::activeThemeId() const{return m_theme?m_theme->activeThemeId():QString();}
QString GameThemeFacade::activeThemeName() const{return m_theme?m_theme->activeThemeName():QString();}
int GameThemeFacade::revision() const{return m_theme?m_theme->revision():0;}
QVariantList GameThemeFacade::themes() const{return m_theme?m_theme->themes():QVariantList();}
int GameThemeFacade::themeCount() const{return m_theme?m_theme->themeCount():0;}
QVariant GameThemeFacade::value(const QString&key)const{return m_theme?m_theme->value(key):QVariant();}
QColor GameThemeFacade::color(const QString&key)const{return m_theme?m_theme->color(key):QColor();}
qreal GameThemeFacade::number(const QString&key)const{return m_theme?m_theme->number(key):0.0;}
QString GameThemeFacade::stringValue(const QString&key)const{return m_theme?m_theme->stringValue(key):QString();}
bool GameThemeFacade::hasValue(const QString&key)const{return m_theme&&m_theme->hasValue(key);}
QStringList GameThemeFacade::keys(const QString&prefix)const{return m_theme?m_theme->keys(prefix):QStringList();}
QVariantMap GameThemeFacade::values(const QString&prefix)const{return m_theme?m_theme->values(prefix):QVariantMap();}
QVariantMap GameThemeFacade::surface(const QString&key)const{return m_theme?m_theme->surface(key):QVariantMap();}
bool GameThemeFacade::isInstalled(const QString&id)const{return m_theme&&m_theme->isInstalled(id);}
QString GameThemeFacade::installedVersion(const QString&id)const{return m_theme?m_theme->installedVersion(id):QString();}
bool GameThemeFacade::applyTheme(const QString&id){return m_theme&&m_theme->applyTheme(id);}
QString GameThemeFacade::lastError() const{return m_theme?m_theme->lastError():QString();}
QStringList GameThemeFacade::capabilities() const{return {QStringLiteral("read_tokens"),QStringLiteral("live_revision"),QStringLiteral("legacy_theme_selection")};}
