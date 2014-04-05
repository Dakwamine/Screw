/*
 * Session.cpp
 *
 *  Created on: Mar 16, 2014
 *      Author: hiepnd
 */

#include "Session.h"
#include <map>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "SessionApple.h"
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "jni/screw/facebook/SessionAndroid.h"
#endif


NS_SCREW_FACEBOOK_BEGIN

#define VALIDATE_STATE(state)   (state == CREATED || state == CREATED_TOKEN_LOADED \
                                || state == OPENING || state == OPENED || state == OPENED_TOKEN_UPDATED \
                                || state == CLOSED_LOGIN_FAILED || state == CLOSED)

static map<int, const char *> __stateString { {Session::State::CREATED, "CREATED"},
                                                    {Session::State::CREATED_TOKEN_LOADED, "CREATED_TOKEN_LOADED"},
                                                    {Session::State::OPENING, "OPENING"},
                                                    {Session::State::OPENED, "OPENED"},
                                                    {Session::State::OPENED_TOKEN_UPDATED, "OPENED_TOKEN_UPDATED"},
                                                    {Session::State::CLOSED_LOGIN_FAILED, "CLOSED_LOGIN_FAILED"},
                                                    {Session::State::CLOSED, "CLOSED"}
                                                  };

Session *Session::_activeSession = nullptr;

Session::Session():_state(INVALID), _appId(""), _initialized(false) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    _impl = new jni::SessionAndroid();
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    _impl = new SessionApple();
#endif
}

Session::~Session() {
    delete _impl;
}


void Session::start() {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	jni::SessionAndroid::start();
#endif
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	SessionApple::start();
#endif
}

void Session::init(State state, const string &appId, list<string> permissions) {
	CCASSERT(!_initialized, "Must be initialized only once");
	CCASSERT(appId != "", "Application ID must not be empty");
    FB_LOG("Session::init - state = %s, appid = %s", __stateString[state], appId.c_str());
#ifdef COCOS2D_DEBUG
    string pstr;
    for (auto i = permissions.begin(); i != permissions.end(); i++) {
    	pstr += string(" ") + (*i);
    }
    FB_LOG("Session::init - permissions = (%s)", pstr.c_str());
#endif

	_initialized = true;
	_state = state;
	_appId = appId;
	_permissions = permissions;
}

Session *Session::getActiveSession() {
    CCASSERT(_activeSession, "Try to get activeSession before it is initialized ?");
	return _activeSession;
}
void Session::initActiveSession(State state, const string &appid, const list<string> &permissions) {
    CCASSERT(VALIDATE_STATE(state), "Invalid state");
    CCASSERT(!_activeSession, "It must be null");
    _activeSession = new Session();
	_activeSession->init(state, appid, permissions);
}

void Session::setStatusCallback(const SessionStatusCallback &callback) {
	_callback = callback;
}

void Session::open(bool allowUi, const list<string> &permissions) {
    _impl->open(allowUi, permissions);
}

void Session::close() {
    _impl->close();
}

void Session::requestReadPermissions(const list<string> &permission) {
    _impl->requestReadPermissions(permission);
}

void Session::requestPublishPermissions(const list<string> &permission) {
    _impl->requestPublishPermissions(permission);
}


Session::State Session::getState() {
	return _state;
}

const string &Session::getAppId() {
	return _appId;
}

bool Session::isOpened() {
	return _state == State::OPENED || _state == State::OPENED_TOKEN_UPDATED;
}

bool Session::isClosed() {
    return _state == State::CLOSED || _state == State::CLOSED_LOGIN_FAILED;
}

void Session::requestReadPermission(const string &permission) {
    list<string> l{permission};
    this->requestReadPermissions(l);
}

void Session::requestPublishPermission(const string &permission) {
    list<string> l{permission};
    this->requestPublishPermissions(l);
}

bool Session::hasPermission(const string &permission) {
    return std::find(_permissions.begin(), _permissions.end(), permission) != _permissions.end();
}

const list<string> &Session::getPermissions() {
    return _permissions;
}

void Session::updateState(Session::State state, const list<string> &permissions) {
	FB_LOG("Session::updateState - state = %s", __stateString[state]);
    CCASSERT(VALIDATE_STATE(state), "Invalid state");
#ifdef COCOS2D_DEBUG
    string pstr;
    for (auto i = permissions.begin(); i != permissions.end(); i++) {
    	pstr += string(" ") + (*i);
    }
    FB_LOG("Session::updateState - permissions = (%s)", pstr.c_str());
#endif
	_state = state;
	_permissions = permissions;
	if (_callback)
		_callback(this);
}

NS_SCREW_FACEBOOK_END
