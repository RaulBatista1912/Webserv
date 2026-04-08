#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <cstring>

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

int main(void) {
    // Payload brut recu (POST body ou QUERY_STRING)
    std::string scoreLine;
    // Ligne finale lisible qui sera ecrite dans score.txt
    std::string recordLine;

    // Methode HTTP actuelle (GET, POST, ...)
    std::string method = std::getenv("REQUEST_METHOD");

    if (method == "POST") {
        const char* lenStr = std::getenv("CONTENT_LENGTH");
        int len = 0;
        if (lenStr)
            len = std::atoi(lenStr);

        if (len > 0) {
            std::string body;
            body.resize(len);
            std::cin.read(&body[0], len);
            scoreLine = body;
        }
    }

    if (scoreLine.empty()) {
        const char* query = std::getenv("QUERY_STRING");
        if (query)
            scoreLine = query;
    }

    // Ouverture du fichier score en mode app.
    std::ofstream scoreFile(getScoreFilePath().c_str(), std::ios::app);

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