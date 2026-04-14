#include "../includes/Session.hpp"

SessionManager::SessionManager() : _ttl(3600){}

SessionManager::~SessionManager(){}

Session* SessionManager::getSession(const std::string& sessionId) {
		std::map<std::string, Session>::iterator it = _sessions.find(sessionId);
		if (it == _sessions.end())
			return NULL;

		time_t now = time(NULL);
		if (it->second.isExpired(now, _ttl)) {
			_sessions.erase(it);
			return NULL;
		}

		it->second.lastAccess = now;
		return &(it->second);
	}

Session& SessionManager::createSession() {
	Session s;
	s.id = generateSessionId();
	s.createdAt = time(NULL);
	s.lastAccess = s.createdAt;
	_sessions[s.id] = s;
	return _sessions[s.id];
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

std::string SessionManager::generateSessionId() {
    std::ostringstream oss;
    oss << time(NULL) << "_"
        << rand() << "_"
        << rand() << "_"
        << rand();
    return oss.str();
}

bool Session::isExpired(time_t now, time_t ttl) const {
	return (now - lastAccess) > ttl;
}
