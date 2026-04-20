#pragma once
#include "Header.hpp"

struct Session {
	std::string _id;
	std::map<std::string, std::string> _data;
	time_t _createdAt;
	time_t _lastAccess;
	bool isExpired(time_t now, time_t ttl) const;
};

struct SessionContext {
	Session* session;
	bool created;
};

class SessionManager {
private:
	std::map<std::string, Session> _sessions;
	time_t _ttl;

public:
	SessionManager();
	~SessionManager();
	Session* getSession(const std::string& sessionId);
	Session& createSession();
	Session& getOrCreateSession(const std::string& sessionId, bool& created);
	void cleanupExpired();
	std::string generateSessionId();
	void deleteSession(const std::string& sessionId);
};
int incrementVisits(Session& session);