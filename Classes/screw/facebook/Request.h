/*
 * Request.h
 *
 *  Created on: Mar 16, 2014
 *      Author: hiepnd
 */

#ifndef REQUEST_H_
#define REQUEST_H_

#include "../macros.h"
#include "GraphObject.h"
#include "cocos2d.h"

USING_NS_CC;
using namespace std;

NS_SCREW_FACEBOOK_BEGIN

typedef std::function<void(int error, GraphObject *result)> RequestCallback;
typedef std::function<void(int error, GraphUser *user)> MeRequestCallback;
typedef std::function<void(int error, const Vector<GraphUser *> &friends)> FriendsRequestCallback;
typedef std::function<void(int error, const Vector<GraphScore *> &scores)> ScoresRequestCallback;
typedef std::function<void(int error, const Vector<GraphRequest *> &requests)> ApprequestsRequestCallback;
typedef std::function<void(int error, bool success)> DeleteRequestCallback;
typedef std::function<void(int error, bool success)> PostScoreRequestCallback;

class Request;

class RequestImpl {
public:
    RequestImpl(){};
    virtual ~RequestImpl(){}
    
    virtual void execute(Request *request) = 0;
};

class Request : public Object {
public:
    enum Method {GET, POST, DELETE};
    
    Request();
    Request(const string &graphPath);
	Request(const string &graphPath, const ValueMap &params, Method method, const RequestCallback &callback);
    static Request *create(const string &graphPath, const ValueMap &params, Method method, const RequestCallback &callback);
	virtual ~Request();
    
    void setGraphPath(const string &graphPath);
    void setParams(const ValueMap &params);
    void setMethod(Method method);
    void setCallback(const RequestCallback &callback);
    
    const string &getGraphPath();
    ValueMap &getParams();
    Method getMethod();
    const RequestCallback &getCallback();
    
    void execute();
    
    //Common requests
    static Request *requestForMe(const MeRequestCallback &callback);
    static Request *requestForFriends(const FriendsRequestCallback &callback);
    static Request *requestForDelete(const string &objectId, const DeleteRequestCallback &callback);
    static Request *requestForScores(const ScoresRequestCallback &callback);
    static Request *requestForMyScore(const ScoresRequestCallback &callback);
    static Request *requestForAppRequests(const ApprequestsRequestCallback &callback);
    static Request *requestForPostScore(long score, const PostScoreRequestCallback &callback);
    
protected:
    Method _method;
    string _graphPath;
    ValueMap _params;
    RequestCallback _callback;
    RequestImpl *_impl;
    
    /* Requests in progress */
    static Vector<Request *> _requests;
};

NS_SCREW_FACEBOOK_END

#endif /* REQUEST_H_ */
