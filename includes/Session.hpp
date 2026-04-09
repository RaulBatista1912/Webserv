#pragma once
#include "Header.hpp"

struct Session {
	std::string id;
	std::map<std::string, std::string> data;
	time_t createdAt;
	time_t lastAccess;
	bool isExpired(time_t now, time_t ttl) const;
};

class SessionManager {
private:
	std::map<std::string, Session> _sessions;
	time_t _ttl;

public:
	SessionManager(time_t ttl = 3600);

	Session* getSession(const std::string& sessionId);
	Session& createSession();
	Session& getOrCreateSession(const std::string& sessionId, bool& created);
	void cleanupExpired();
	std::string generateSessionId();
};