#include "util.hpp"

namespace kvantum {

vector<string> std_string_split(string str)
{
    vector<string> ret;
    for (int i = 0; i < str.size(); i++) {
        string s;
        auto instr = false;
        while (i < str.size() && (instr || !isspace(str[i]))) {
            s += str[i];
            if (str[i] == '\'')
                instr = !instr;
            i++;
        }
        //str[i] is whitespace
        if (!s.empty())
            ret.push_back(s);
    }
    return ret;
}

} // namespace kvantum
