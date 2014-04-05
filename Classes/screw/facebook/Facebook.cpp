/*
 * Facebook.cpp
 *
 *  Created on: Mar 16, 2014
 *      Author: hiepnd
 */

#include "Facebook.h"

NS_SCREW_FACEBOOK_BEGIN

//Data key
static const char *FacebookDataProfilesKey              = "__profiles__";
static const char *FacebookDataProfilesTimestampKey     = "__profiles_timestamp__";
static const char *FacebookDataPhotosTimestampKey       = "__photos_timestamp__";
static const char *FacebookDataRequestTimestampKey      = "__request_timestamp__";
static const char *FacebookDataUserIDKey                = "__user_id__";
static const char *FacebookDataLocalUserKey             = "__local_user__";
static const char *FacebookDataFacebookScoreKey         = "__fb_score__";
static const char *FacebookDataDirtyScoreKey            = "__dirty_score__";
static const char *FacebookDataAllTimeHightScoreKey     = "__all_time_hight_score__";

#define FACEBOOK_PROFILE_KEY(uid)           screw::data::PathBuilder::create(FacebookDataProfilesKey)->append(uid)->build()
#define FACEBOOK_PROFILE_TIMESTAMP_KEY(uid) screw::data::PathBuilder::create(FacebookDataProfilesTimestampKey)->append(uid)->build()
#define FACEBOOK_PHOTO_TIMESTAMP_KEY(uid)   screw::data::PathBuilder::create(FacebookDataPhotosTimestampKey)->append(uid)->build()
#define FACEBOOK_REQUEST_TIMESTAMP_KEY(uid) screw::data::PathBuilder::create(FacebookDataRequestTimestampKey)->append(uid)->build()

#define RETURN_IF_STATE_SET(bit, msg) if (_loadingBits & (bit)) {CCLOG("%s", msg); return;}

#define FB_SET_FETCHING_STATE(bit)      _loadingBits |= bit;
#define FB_CLEAR_FETCHING_STATE(bit)    _loadingBits &= ~bit;

Facebook::Facebook() {
	// TODO Auto-generated constructor stub
    
    _loadingBits = 0x00;
}

Facebook::~Facebook() {
	// TODO Auto-generated destructor stub
}


#pragma mark Fetch
void Facebook::fetchUserDetails(const MeRequestCallback &handler) {
    RETURN_IF_STATE_SET(FacebookFetchingUserDetail, "Facebook::fetchUserDetails - another request is in progress");
    
    FB_SET_FETCHING_STATE(FacebookFetchingUserDetail);
    Request::requestForMe([=](int error, GraphUser *user){
        if (!error && user) {
            this->didFetchUserDetail(user);
        }
        if (handler) {
            handler(error, user);
        }
        FB_CLEAR_FETCHING_STATE(FacebookFetchingUserDetail);
    })->execute();
}

void Facebook::fetchFriends(const FriendsRequestCallback &handler) {
    RETURN_IF_STATE_SET(FacebookFetchingFriends, "Facebook::fetchFriends - another request is in progress");
    
    FB_SET_FETCHING_STATE(FacebookFetchingFriends);
    Request::requestForFriends([=](int error, const Vector<GraphUser *> &friends){
        if (!error) {
            this->didFetchFriends(friends);
        }
        if (handler) {
            handler(error, friends);
        }
        FB_CLEAR_FETCHING_STATE(FacebookFetchingFriends);
    })->execute();
}

void Facebook::fetchScores(const ScoresRequestCallback &handler){
    RETURN_IF_STATE_SET(FacebookFetchingHighScores, "Facebook::fetchScores - another request is in progress");
    
    FB_SET_FETCHING_STATE(FacebookFetchingHighScores);
    Request::requestForScores([=](int error, const Vector<GraphScore *> &scores){
        if (!error) {
            this->didFetchScores(scores);
        }
        if (handler) {
            handler(error, scores);
        }
        FB_CLEAR_FETCHING_STATE(FacebookFetchingHighScores);
    })->execute();
}

void Facebook::postScore(long score) {
    this->setDirtyScore(score);
    Request::requestForMyScore([=](int error, const Vector<GraphScore *> &scores){
        if (!error) {
            if (scores.size()) {
                if (scores.at(0)->getScore() < score) {
                    this->doPostScore(score);
                } else {
                    this->clearDirtyScore();
                }
            } else {
                //No score
                this->doPostScore(score);
            }
        }
    })->execute();
}

void Facebook::doPostScore(long score) {
    Request::requestForPostScore(score, [=](int error, bool success){
        if (success) {
            this->clearDirtyScore();
        }
    })->execute();
}

#pragma mark Score
void Facebook::setDirtyScore(long score) {
    _data->set(FacebookDataDirtyScoreKey, score);
}

long Facebook::getDirtyScore() {
    return _data->getLong(FacebookDataDirtyScoreKey);
}

void Facebook::clearDirtyScore() {
    _data->clear(FacebookDataDirtyScoreKey);
}

#pragma mark Private Save
void Facebook::saveUserDetail(GraphUser *user) {
    //Get current score
    Value data = user->getData();
    long score = _data->getLong(screw::data::PathBuilder::create(FacebookDataProfilesKey)->append(user->getId())->append(GraphUser::SCORE)->build());
    if (score != 0) {
        ValueSetter::set(data, GraphUser::SCORE, score);
    }
    ValueSetter::set(data, GraphUser::INSTALLED, true);
    _data->set(FACEBOOK_PROFILE_KEY(user->getId()), data);
    _data->set(FacebookDataUserIDKey, user->getId());
}

void Facebook::saveFriend(GraphUser *user) {
    Value data = user->getData();
    long score = _data->getLong(screw::data::PathBuilder::create(FacebookDataProfilesKey)->append(user->getId())->append(GraphUser::SCORE)->build());
    ValueSetter::set(data, GraphUser::SCORE, score);
    _data->set(FACEBOOK_PROFILE_KEY(user->getId()), data);
}

void Facebook::didFetchUserDetail(GraphUser *user) {
    
    _state = State::LOGIN_WITH_ID;
}

void Facebook::didFetchFriends(const Vector<GraphUser *> &friends) {
    for (auto f : friends) {
        this->saveFriend(f);
    }
    _data->save();
}

void Facebook::didFetchScores(const Vector<GraphScore *> &scores) {
    for (auto s : scores) {
        GraphUser *user = s->getUser();
        Value &data = _data->get(FACEBOOK_PROFILE_KEY(user->getId()));
        if (!data.isNull()) {
            //User existed, update score
            ValueSetter::set(data, GraphUser::SCORE, s->getScore());
            if (getUser()->getId() == user->getId()) {
                
            }
        } else {
            ValueSetter::set(data, GraphUser::INSTALLED, true);
            _data->set(FACEBOOK_PROFILE_KEY(user->getId()), data);
        }
    }
}

void Facebook::didAuthorizeNewUser(GraphUser *user) {
    
}


NS_SCREW_FACEBOOK_END