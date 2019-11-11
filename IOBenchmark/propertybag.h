#pragma once

#include <any>
#include <map>
#include <string>
#include <algorithm>

class IPropertyBagRead
{
public:
    virtual bool TryGetInt(const std::string& str, int* ptr) const = 0;
};

class CPropertyBag : public  IPropertyBagRead
{
private:
    std::map<std::string, std::any> properties;
    //std::vector<std::tuple<std::string, std::any>> properties;
public:
    void AddItem_String(const std::string& str, std::string s)
    {
        this->properties[str] = std::any(s);
    }

    void AddItem_Bool(const std::string& str, bool b)
    {
        this->properties[str] = std::any(b);
    }

    void AddItem_Int32(const std::string& str, int v)
    {
        this->properties[str] = std::any(v);
    }

    bool TryGetInt(const std::string& str, int* ptr) const
    {
        const auto i = this->properties.find(str);
        if (i != this->properties.cend())
        {
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

        return false;
        /*auto i = std::find(this->properties.cbegin(), this->properties.cend(), [&](const std::tuple<std::string, std::any>& e) {return std::get<0>(e) == str; });
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

        return true;*/
    }
};
