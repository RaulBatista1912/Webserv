#include "../includes/Session.hpp"

SessionManager::SessionManager() : _ttl(3600){}

SessionManager::~SessionManager(){}

//Get the session id
Session* SessionManager::getSession(const std::string& sessionId) {
	std::map<std::string, Session>::iterator it = _sessions.find(sessionId);
	if (it == _sessions.end())
		return NULL;

	time_t now = time(NULL);
	if (it->second.isExpired(now, _ttl)) {
		_sessions.erase(it);
		return NULL;
	}

	it->second._lastAccess = now;
	return &(it->second);
}

//create the session with an id, date, last acess and add to the map sessions
Session& SessionManager::createSession() {
	Session s;
	s._id = generateSessionId();
	s._createdAt = time(NULL);
	s._lastAccess = s._createdAt;
	s._visits = 0;
	_sessions[s._id] = s;
	return _sessions[s._id];
}

Session& SessionManager::getOrCreateSession(const std::string& sessionId, bool& created) {
	Session* existing = getSession(sessionId);
	if (existing) {
		created = false;
		return *existing;
	}
	created = true;
	return createSession();
}

//Check each session if it expired
void SessionManager::cleanupExpired() {
	time_t now = time(NULL);
	for (std::map<std::string, Session>::iterator it = _sessions.begin();
			it != _sessions.end();) {
		if (it->second.isExpired(now, _ttl))
			_sessions.erase(it++);
		else
			++it;
	}
}

//create the id
std::string SessionManager::generateSessionId() {
	std::ostringstream oss;
	oss << time(NULL) << "_"
		<< rand() << "_"
		<< rand() << "_"
		<< rand();
	return oss.str();
}

//Check if the session expired
bool Session::isExpired(time_t now, time_t ttl) const {
	return (now - _lastAccess) > ttl;
}

void SessionManager::deleteSession(const std::string& sessionId) {
	_sessions.erase(sessionId);
}

int incrementVisits(Session& session) {
	int visits = 0;

	if (session._data.find("visits") != session._data.end())
		visits = std::atoi(session._data["visits"].c_str());

	visits++;
	session._visits = visits;
	std::ostringstream vs;
	vs << visits;
	session._data["visits"] = vs.str();

	return visits;
}