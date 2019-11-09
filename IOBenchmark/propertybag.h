#pragma once

#include <any>
#include <vector>
#include <string>
#include <algorithm>

class CPropertyBag
{
private:
    std::vector<std::tuple<std::string, std::any>> properties;
public:

    void AddItem_Int32(const std::string& str, int v)
    {
        this->properties.push_back(std::make_tuple(str, std::any(v)));
    }

    bool TryGetInt(const std::string& str, int* ptr) const
    {
        auto i = std::find(this->properties.cbegin(), this->properties.cend(), str);
        if (i == this->properties.cend())
        {
            return false;
        }

        try
        {
            int v = std::any_cast<int>(std::get<1>(*i));
            if (ptr != nullptr)
            {
                *ptr = v;
            }
        }
        catch (const std::bad_any_cast&)
        {
            return false;
        }

        return true;
    }
}
};
