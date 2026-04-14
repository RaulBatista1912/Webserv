#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <cstring>
#include <cerrno>
#include <sstream>

/*
    Convertit un caractere hexadecimal en entier.
    Retourne -1 si le char n'est pas hexa.
*/
static int hexToInt(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F')
        return 10 + (c - 'A');
    return -1;
}

/*
    Decode une chaine URL encodee.
    - '+' espace
    - '%XX' represente un octet hexadecimal
*/
static std::string urlDecode(const std::string& s)
{
    std::string out;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') {
            out += ' ';
        }
        else if (s[i] == '%' && i + 2 < s.size()) {
            int hi = hexToInt(s[i + 1]);
            int lo = hexToInt(s[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out += static_cast<char>(hi * 16 + lo);
                i += 2;
            }
            else {
                out += s[i];
            }
        }
        else {
            out += s[i];
        }
    }
    return out;
}

/*
    "name=alice&score=1200"
    Si key="name"  -> retourne "alice"
    Si key="score" -> retourne "1200"
*/
static std::string extractParam(const std::string& payload, const std::string& key)
{
    std::size_t pos = 0;
    while (pos < payload.size()) {
        std::size_t amp = payload.find('&', pos);
        std::string part = payload.substr(pos, amp == std::string::npos ? std::string::npos : amp - pos);

        std::size_t eq = part.find('=');
        if (eq != std::string::npos) {
            std::string k = part.substr(0, eq);
            std::string v = part.substr(eq + 1);
            if (k == key)
                return urlDecode(v);
        }

        if (amp == std::string::npos)
            break;
        pos = amp + 1;
    }
    return "";
}

static std::string getScoreFilePath()
{
    const char* script = std::getenv("SCRIPT_FILENAME");
    if (!script)
        return "score.txt";

    std::string scriptPath = script;
    std::size_t slash = scriptPath.find_last_of('/');
    if (slash == std::string::npos)
        return "score.txt";

    return scriptPath.substr(0, slash + 1) + "score.txt";
}

static std::string extractFromRecordLine(const std::string& line,
                                         const std::string& key)
{
    std::string needle = key + "=";
    std::size_t pos = line.find(needle);
    if (pos == std::string::npos)
        return "";

    pos += needle.size();
    std::size_t end = line.find(' ', pos);
    if (end == std::string::npos)
        return line.substr(pos);
    return line.substr(pos, end - pos);
}

static bool findBestScoreByName(const std::string& scorePath,
                                const std::string& playerName,
                                std::string& bestScore)
{
    std::ifstream in(scorePath.c_str());
    if (!in)
        return false;

    bool found = false;
    int best = 0;
    std::string line;

    while (std::getline(in, line)) {
        std::string name = extractFromRecordLine(line, "name");
        if (name != playerName)
            continue;

        int score = std::atoi(extractFromRecordLine(line, "score").c_str());
        if (!found || score > best) {
            best = score;
            found = true;
        }
    }

    if (!found)
        return false;

    std::ostringstream oss;
    oss << best;
    bestScore = oss.str();
    return true;
}

int main(void) {
    std::string scorePath = getScoreFilePath();
    // Payload brut recu
    std::string scoreLine;
    // Ligne qui sera ecrite dans score.txt
    std::string recordLine;
    // Methode HTTP (GET, POST, ...)
    const char* method = std::getenv("REQUEST_METHOD");
    const char* query = std::getenv("QUERY_STRING");
    std::string queryString = query ? query : "";

    // Mode recherche: GET /save-score.cgi?name=alice
    if (method && std::strcmp(method, "GET") == 0) {
        std::string wantedName = extractParam(queryString, "name");
        std::string maybeScore = extractParam(queryString, "score");

        if (!wantedName.empty() && maybeScore.empty()) {
            std::string bestScore;
            bool found = findBestScoreByName(scorePath, wantedName, bestScore);

            std::cout << "Content-Type: text/plain\r\n";
            std::cout << "Cache-Control: no-store\r\n";
            std::cout << "Pragma: no-cache\r\n\r\n";

            if (found)
                std::cout << "name=" << wantedName << " score=" << bestScore << "\n";
            else
                std::cout << "name=" << wantedName << " score=not_found\n";
            return 0;
        }
    }

    if (method && std::strcmp(method, "POST") == 0) {
        const char* lenStr = std::getenv("CONTENT_LENGTH");
        int len = 0;
        if (lenStr)
            len = std::atoi(lenStr);

        if (len > 0) {
            std::string body;
            body.resize(len);
            std::cin.read(&body[0], len);
            std::streamsize readCount = std::cin.gcount();
            if (readCount > 0) {
                body.resize(static_cast<std::size_t>(readCount));
                scoreLine = body;
            }
        }
    }

    if (scoreLine.empty()) {
        if (query)
            scoreLine = query;
    }

    // Ouverture du fichier score en mode app.
    std::ofstream scoreFile(scorePath.c_str(), std::ios::app);
    if (!scoreFile) {
        std::cerr << "[save-score.cgi] open failed path='" << scorePath
                  << "' error='" << std::strerror(errno) << "'" << std::endl;
        std::cout << "Status: 500 Internal Server Error\r\n";
        std::cout << "Content-Type: text/plain\r\n";
        std::cout << "Cache-Control: no-store\r\n";
        std::cout << "Pragma: no-cache\r\n\r\n";
        std::cout << "failed to open score file: " << scorePath
                  << " (" << std::strerror(errno) << ")\n";
        return 1;
    }

    // Extraction des champs utiles depuis le payload (name=...&score=...)
    std::string playerName = extractParam(scoreLine, "name");
    std::string playerScore = extractParam(scoreLine, "score");

    // Valeurs par defaut si un champ manque
    if (playerName.empty())
        playerName = "unknown";
    if (playerScore.empty())
        playerScore = "0";

    //stocke dans score.txt
    recordLine = "name=" + playerName + " score=" + playerScore;

    if (!scoreLine.empty()) {
        // (debug)
        std::cerr << "[save-score.cgi] " << recordLine << std::endl;

        std::cout << "Content-Type: text/plain\r\n";
        std::cout << "Cache-Control: no-store\r\n";
        std::cout << "Pragma: no-cache\r\n\r\n";
        std::cout << "saved " << recordLine << "\n";

        // Ecriture du score dans le fichier
        if (scoreFile)
            scoreFile << recordLine << std::endl;
    }
    else {
        // 0 donnee recue
        std::cerr << "[save-score.cgi] score=0" << std::endl;
        std::cout << "Content-Type: text/plain\r\n";
        std::cout << "Cache-Control: no-store\r\n";
        std::cout << "Pragma: no-cache\r\n\r\n";
        std::cout << "saved score=0\n";

        if (scoreFile)
            scoreFile << "score=0" << std::endl;
    }

    return 0;
}