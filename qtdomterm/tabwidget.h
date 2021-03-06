/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QtWebEngineWidgets/QWebEngineFullScreenRequest>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QShortcut>

QT_BEGIN_NAMESPACE
class QWebEngineDownloadItem;
class QWebEngineProfile;
class QWebEngineView;
QT_END_NAMESPACE

class Backend;
#include "processoptions.h"

/*
    Tab bar with a few more features such as a context menu and shortcuts
 */
class TabBar : public QTabBar
{
    Q_OBJECT

signals:
    void newTab();
    void cloneTab();
    void closeTab(int index);
    void closeOtherTabs(int index);
    void tabMoveRequested(int fromIndex, int toIndex);

public:
    TabBar(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

private slots:
    void selectTabAction();
    void closeTab();
    void closeOtherTabs();
    void contextMenuRequested(const QPoint &position);

private:
    QList<QShortcut*> m_tabShortcuts;
    friend class TabWidget;

    QPoint m_dragStartPos;
    int m_dragCurrentIndex;
};

#include <QWebEnginePage>

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE
class WebView;
/*!
    A proxy object that connects a single browser action
    to one child webpage action at a time.

    Example usage: used to keep the main window stop action in sync with
    the current tabs webview's stop action.
 */
class WebActionMapper : public QObject
{
    Q_OBJECT

public:
    WebActionMapper(QAction *root, QWebEnginePage::WebAction webAction, QObject *parent);
    QWebEnginePage::WebAction webAction() const;
    void addChild(QAction *action);
    void updateCurrent(QWebEnginePage *currentParent);

private slots:
    void rootTriggered();
    void childChanged();
    void rootDestroyed();
    void currentDestroyed();

private:
    QWebEnginePage *m_currentParent;
    QAction *m_root;
    QWebEnginePage::WebAction m_webAction;
};

#include <QtCore/QUrl>
#include <QtWidgets/QTabWidget>

class FullScreenNotification;

QT_BEGIN_NAMESPACE
class QCompleter;
class QMenu;
class QStackedWidget;
QT_END_NAMESPACE

/*!
    TabWidget that contains WebViews and a stack widget of associated line edits.

    Connects up the current tab's signals to this class's signal and uses WebActionMapper
    to proxy the actions.
 */
class TabWidget : public QTabWidget
{
    Q_OBJECT

signals:
    // tab widget signals
    void loadPage(const QString &url);
    void tabsChanged();
    void lastTabClosed();

    // current tab signals
    void setCurrentTitle(const QString &title);
    void linkHovered(const QString &link);
    void loadProgress(int progress);
    void geometryChangeRequested(const QRect &geometry);
    void menuBarVisibilityChangeRequested(bool visible);
#if defined(QWEBENGINEPAGE_PRINTREQUESTED)
    void printRequested(QWebEngineFrame *frame);
#endif

public:
    TabWidget(QWidget *parent = 0);
    ~TabWidget();
    void clear();
    void addWebAction(QAction *action, QWebEnginePage::WebAction webAction);
    QAction *newTabAction() const;
    QAction *closeTabAction() const;
    QAction *saveAsAction() const { return m_saveAsAction; }
    QAction *changeCaretAction() const { return m_changeCaretAction; }
    QAction *recentlyClosedTabsAction() const;
    QAction *nextTabAction() const;
    QAction *previousTabAction() const;

    WebView *currentWebView() const;
    Backend *currentBackend() const;
    WebView *webView(int index) const;
    int webViewIndex(WebView *webView) const;

    void setProfile(QWebEngineProfile *profile);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

public slots:
    void loadUrlInCurrentTab(const QUrl &url);
    WebView *newTab(bool makeCurrent = true);
    WebView *newTab(QSharedDataPointer<ProcessOptions> processOptions, bool makeCurrent = true);
    void cloneTab();
    void requestCloseTab();
    void requestCloseTab(int index);
    void requestSaveAs();
    void requestChangeCaret(bool);
    void closeTab(int index);
    void closeOtherTabs(int index);
    void nextTab();
    void previousTab();

private slots:
    void currentChanged(int index);
    void aboutToShowRecentTabsMenu();
    void aboutToShowRecentTriggeredAction(QAction *action);
    //void downloadRequested(QWebEngineDownloadItem *download);
    void webViewLoadStarted();
    void webViewIconChanged(const QIcon &icon);
    void webViewTitleChanged(const QString &title);
    void windowCloseRequested();
    void moveTab(int fromIndex, int toIndex);
    void fullScreenRequested(QWebEngineFullScreenRequest request);
    void handleTabBarDoubleClicked(int index);

private:
    void setupPage(QWebEnginePage* page);

    QAction *m_recentlyClosedTabsAction;
    QAction *m_newTabAction;
    QAction *m_closeTabAction;
    QAction *m_nextTabAction;
    QAction *m_previousTabAction;
    QAction *m_saveAsAction;
    QAction *m_changeCaretAction;

    QMenu *m_recentlyClosedTabsMenu;
    static const int m_recentlyClosedTabsSize = 10;
    QList<QUrl> m_recentlyClosedTabs;
    QList<WebActionMapper*> m_actions;

    TabBar *m_tabBar;
    QWebEngineProfile *m_profile;
    QWebEngineView *m_fullScreenView;
    FullScreenNotification *m_fullScreenNotification;
};

#endif // TABWIDGET_H
