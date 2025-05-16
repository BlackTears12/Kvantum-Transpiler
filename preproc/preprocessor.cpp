#include "preproc/preprocessor.hpp"
#include <algorithm>
#include <cstring>
#include <iterator>

namespace kvantum {

void Preprocessor::processFile(const string filename)
{
    includes.push_front(filename);
    std::ifstream is;
    is.open(filename);
    if (is.fail())
        throw std::invalid_argument(std::strerror(errno));
    std::string expr;
    while (std::getline(is, expr)) {
        string::iterator iter = std::find(expr.begin(), expr.end(), ' ');
        if (iter >= expr.end() - 1 || string(expr.begin(), iter) != "import")
            continue;
        string file(iter + 1, expr.end());
        if (!contains(includes.begin(), includes.end(), file))
            processFile(file);
    }
}

deque<string>& Preprocessor::getIncludes()
{
    return includes;
}

} // namespace kvantum
