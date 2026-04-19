#include "../includes/ServerConfig.hpp"

const Location* ServerConfig::findLocation(const std::string& path) const
{
	const Location* best = NULL;
	const Location* root = NULL;

	for (size_t i = 0; i < locations.size(); ++i) {
		const Location& loc = locations[i];

		if (loc.path == "/")
			root = &loc;

		if (path.compare(0, loc.path.size(), loc.path) == 0) {
			if (!best || loc.path.size() > best->path.size())
				best = &loc;
		}
	}
	if (best)
		return best;
	return root;
}