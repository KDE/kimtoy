#ifndef SCIM_KCONFIG_CONFIG_H
#define SCIM_KCONFIG_CONFIG_H

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_C_STDIO

#define ENABLE_DEBUG 9

#include <scim.h>

class KConfig;

using namespace scim;

class KConfigConfig : public ConfigBase
{
public:
    explicit KConfigConfig();
    virtual ~KConfigConfig();
    virtual bool valid() const;
    virtual String get_name() const;
    virtual bool read(const String& key, String *ret) const;
    virtual bool read(const String& key, int *ret) const;
    virtual bool read(const String& key, double *ret) const;
    virtual bool read(const String& key, bool *ret) const;
    virtual bool read(const String& key, std::vector <String> *ret) const;
    virtual bool read(const String& key, std::vector <int> *ret) const;
    virtual bool write(const String& key, const String& value);
    virtual bool write(const String& key, int value);
    virtual bool write(const String& key, double value);
    virtual bool write(const String& key, bool value);
    virtual bool write(const String& key, const std::vector <String>& value);
    virtual bool write(const String& key, const std::vector <int>& value);
    virtual bool flush();
    virtual bool erase(const String& key);
    virtual bool reload();
private:
    KConfig* m_config;
};

#endif // SCIM_KCONFIG_CONFIG_H
