#ifndef SCIM_PANEL_H
#define SCIM_PANEL_H

#include <dbus-c++/dbus.h>
#include "inputmethod_adaptor.h"
#include "impanel_proxy.h"

class Panel : public org::kde::kimpanel::inputmethod_adaptor,
              public org::kde::impanel_proxy,
              public DBus::IntrospectableAdaptor,
              public DBus::ObjectAdaptor,
              public DBus::IntrospectableProxy,
              public DBus::ObjectProxy
{
public:
    explicit Panel(DBus::Connection &connection);
    virtual ~Panel();
    virtual void MovePreeditCaret(const int32_t& pos);
    virtual void SelectCandidate(const int32_t& index);
    virtual void LookupTablePageUp();
    virtual void LookupTablePageDown();
    virtual void TriggerProperty(const std::string& key);
    virtual void PanelCreated();
    virtual void Exit();
    virtual void ReloadConfig();
    virtual void Configure();
private:
};

#endif // SCIM_PANEL_H
