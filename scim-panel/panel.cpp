#include "panel.h"
#include <iostream>

Panel::Panel(DBus::Connection &connection)
: DBus::ObjectAdaptor(connection, "/kimpanel"),
DBus::ObjectProxy(connection, "/org/kde/impanel", "org.kde.impanel")
{
}

Panel::~Panel()
{
}

void Panel::MovePreeditCaret(const int32_t& pos)
{
}

void Panel::SelectCandidate(const int32_t& index)
{
}

void Panel::LookupTablePageUp()
{
}

void Panel::LookupTablePageDown()
{
}

void Panel::TriggerProperty(const std::string& key)
{
    std::cerr << key << std::endl;
}

void Panel::PanelCreated()
{
}

void Panel::Exit()
{
}

void Panel::ReloadConfig()
{
}

void Panel::Configure()
{
}
