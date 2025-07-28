#include "../includes/Utils.hpp"

std::string load_html_file(const std::string& path)
{
    std::ifstream file(path.c_str());
    if (!file)
        return "";

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
