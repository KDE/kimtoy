#include "panel.h"
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define Uses_SCIM_CONFIG
#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_PANEL_AGENT
#include <scim.h>

using namespace scim;

static PanelAgent* panel_agent = 0;
static ConfigModule* _config_module = 0;
static ConfigPointer _config;

static std::vector<HelperInfo> _helper_list;

// static std::map<int, PropertyList> helper_props_map;

static pthread_t panel_agent_tid = 0;

DBus::BusDispatcher dispatcher;
Panel* panel = 0;


static void slot_transaction_start(void);
static void slot_transaction_end(void);
static void slot_reload_config(void);
static void slot_turn_on(void);
static void slot_turn_off(void);
static void slot_update_screen(int screen);
static void slot_update_spot_location(int x, int y);
static void slot_update_factory_info(const PanelFactoryInfo& info);
static void slot_show_help(const String& help);
static void slot_show_factory_menu(const std::vector<PanelFactoryInfo>& menu);
static void slot_show_preedit_string(void);
static void slot_show_aux_string(void);
static void slot_show_lookup_table(void);
static void slot_hide_preedit_string(void);
static void slot_hide_aux_string(void);
static void slot_hide_lookup_table(void);
static void slot_update_preedit_string(const String& str, const AttributeList& attrs);
static void slot_update_preedit_caret(int caret);
static void slot_update_aux_string(const String& str, const AttributeList& attrs);
static void slot_update_lookup_table(const LookupTable& table);
static void slot_register_properties(const PropertyList& props);
static void slot_update_property(const Property& prop);
static void slot_register_helper_properties(int id, const PropertyList& props);
static void slot_update_helper_property(int id, const Property& prop);
static void slot_register_helper(int id, const HelperInfo& helper);
static void slot_remove_helper(int id);
static void slot_lock(void);
static void slot_unlock(void);

static bool initialize_panel_agent(const String& config, const String& display, bool resident)
{
    panel_agent = new PanelAgent();

    if (!panel_agent->initialize(config, display, resident))
        return false;

    panel_agent->signal_connect_transaction_start(slot(slot_transaction_start));
    panel_agent->signal_connect_transaction_end(slot(slot_transaction_end));
    panel_agent->signal_connect_reload_config(slot(slot_reload_config));
    panel_agent->signal_connect_turn_on(slot(slot_turn_on));
    panel_agent->signal_connect_turn_off(slot(slot_turn_off));
    panel_agent->signal_connect_update_screen(slot(slot_update_screen));
    panel_agent->signal_connect_update_spot_location(slot(slot_update_spot_location));
    panel_agent->signal_connect_update_factory_info(slot(slot_update_factory_info));
    panel_agent->signal_connect_show_help(slot(slot_show_help));
    panel_agent->signal_connect_show_factory_menu(slot(slot_show_factory_menu));
    panel_agent->signal_connect_show_preedit_string(slot(slot_show_preedit_string));
    panel_agent->signal_connect_show_aux_string(slot(slot_show_aux_string));
    panel_agent->signal_connect_show_lookup_table(slot(slot_show_lookup_table));
    panel_agent->signal_connect_hide_preedit_string(slot(slot_hide_preedit_string));
    panel_agent->signal_connect_hide_aux_string(slot(slot_hide_aux_string));
    panel_agent->signal_connect_hide_lookup_table(slot(slot_hide_lookup_table));
    panel_agent->signal_connect_update_preedit_string(slot(slot_update_preedit_string));
    panel_agent->signal_connect_update_preedit_caret(slot(slot_update_preedit_caret));
    panel_agent->signal_connect_update_aux_string(slot(slot_update_aux_string));
    panel_agent->signal_connect_update_lookup_table(slot(slot_update_lookup_table));
    panel_agent->signal_connect_register_properties(slot(slot_register_properties));
    panel_agent->signal_connect_update_property(slot(slot_update_property));
    panel_agent->signal_connect_register_helper_properties(slot(slot_register_helper_properties));
    panel_agent->signal_connect_update_helper_property(slot(slot_update_helper_property));
    panel_agent->signal_connect_register_helper(slot(slot_register_helper));
    panel_agent->signal_connect_remove_helper(slot(slot_remove_helper));
    panel_agent->signal_connect_lock(slot(slot_lock));
    panel_agent->signal_connect_unlock(slot(slot_unlock));

    panel_agent->get_helper_list(_helper_list);

    return true;
}

static void* panel_agent_run(void* data)
{
    panel_agent->run();

    pthread_detach(pthread_self());

    return NULL;
}

static bool run_panel_agent()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1024);

    int ret = pthread_create(&panel_agent_tid, &attr, panel_agent_run, NULL);

    pthread_attr_destroy(&attr);

    return (ret == 0);
}

static void start_auto_start_helpers()
{
    // Add Helper object items
    for (size_t i = 0; i < _helper_list.size(); ++i) {
        if (_helper_list[i].option & SCIM_HELPER_AUTO_START) {
            panel_agent->start_helper(_helper_list[i].uuid);
        }
    }
}

static String AttrList2String(const AttributeList& attr_list)
{
    String result;
    AttributeList::const_iterator it = attr_list.begin();
    for (; it != attr_list.end(); ++it) {
        std::stringstream ss;
        ss << (int)it->get_type() << ':';
        ss << it->get_start() << ':';
        ss << it->get_length() << ':';
        ss << it->get_value() << ';';
        result += ss.str();
    }
    return result;
}

static String Property2String(const Property& prop)
{
    int state = 0;
    if (prop.active()) state |= SCIM_PROPERTY_ACTIVE;
    if (prop.visible()) state |= SCIM_PROPERTY_VISIBLE;

    std::stringstream ss;
    ss << prop.get_key() << ':';
    ss << prop.get_label() << ':';
    ss << prop.get_icon() << ':';
    ss << prop.get_tip() << ':';
    ss << state;
    return ss.str();
}

static String PanelFactoryInfo2String(const PanelFactoryInfo& info)
{
    std::stringstream ss;
    ss << "/Factory/" << info.uuid << ':';
    ss << info.name << ':';
    ss << info.icon << ':';
    ss << info.lang;
    return ss.str();
}

static void slot_transaction_start(void)
{
    std::cerr << "slot_transaction_start\n";
//     _transaction_lock.lock();
}

static void slot_transaction_end(void)
{
    std::cerr << "slot_transaction_end\n";
//     _transaction_lock.unlock();
}

static void slot_reload_config(void)
{
    std::cerr << "slot_reload_config\n";
    _config->reload();
}

static void slot_turn_on(void)
{
    std::cerr << "slot_turn_on\n";
    panel->Enable(true);
}

static void slot_turn_off(void)
{
    std::cerr << "slot_turn_off\n";
    panel->Enable(false);
}

static void slot_update_screen(int num)
{
    std::cerr << "slot_update_screen\n";
    //FIXME
//     qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::UP_SCREEN,
//                     QVariantList() << num));
}

static void slot_update_factory_info(const PanelFactoryInfo &info)
{
    std::cerr << "slot_update_factory_info\n";
    //FIXME update logo prop
//     qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::UP_FACTORY_INFO,
//                     QVariantList() << QVariant::fromValue(info)));
}

static void slot_show_help(const String& help)
{
    panel->ExecDialog(help);
}

static void slot_show_factory_menu(const std::vector<PanelFactoryInfo>& factories)
{
    std::vector<String> list;
    std::vector<PanelFactoryInfo>::const_iterator it = factories.begin();
    for (; it != factories.end(); ++it) {
        list.push_back(PanelFactoryInfo2String(*it));
    }
    panel->ExecMenu(list);
}

static void slot_update_spot_location(int x, int y)
{
    panel->UpdateSpotLocation(x, y);
}

static void slot_show_preedit_string(void)
{
    panel->ShowPreedit(true);
}

static void slot_show_aux_string(void)
{
    panel->ShowAux(true);
}

static void slot_show_lookup_table(void)
{
    panel->ShowLookupTable(true);
}

static void slot_hide_preedit_string(void)
{
    panel->ShowPreedit(false);
}

static void slot_hide_aux_string(void)
{
    panel->ShowAux(false);
}

static void slot_hide_lookup_table(void)
{
    panel->ShowLookupTable(false);
}

static void slot_update_preedit_string(const String& str, const AttributeList& attrs)
{
    panel->UpdatePreeditText(str, AttrList2String(attrs));
}

static void slot_update_preedit_caret(int caret)
{
    panel->UpdatePreeditCaret(caret);
}

static void slot_update_aux_string(const String& str, const AttributeList& attrs)
{
    panel->UpdateAux(str, AttrList2String(attrs));
}

static void slot_update_lookup_table(const LookupTable& table)
{
    std::vector<String> labels;
    std::vector<String> candidates;
    std::vector<String> attrs;
    int current_page_size = table.get_current_page_size();
    for (int i = 0; i < current_page_size; ++i) {
        labels.push_back(utf8_wcstombs(table.get_candidate_label(i)));
    }
    for (int i = 0; i < current_page_size; ++i) {
        candidates.push_back(utf8_wcstombs(table.get_candidate_in_current_page(i)));
        attrs.push_back(AttrList2String(table.get_attributes_in_current_page(i)));
    }
    bool hasprev = table.get_current_page_start();
    bool hasnext = hasprev + current_page_size < table.number_of_candidates();
    panel->UpdateLookupTable(labels, candidates, attrs, hasprev, hasnext);
}

static void slot_register_properties(const PropertyList& props)
{
    // TODO logo prop
    std::vector<String> list;
    PropertyList::const_iterator it = props.begin();
    PropertyList::const_iterator end = props.end();
    PropertyList::const_iterator next = it;
    while (it != end) {
        ++next;
        if (next == end || !next->is_a_leaf_of(*it)) {
            list.push_back(Property2String(*it));
            it = next;
        }
    }
    panel->RegisterProperties(list);
}

static void slot_update_property(const Property &prop)
{
    panel->UpdateProperty(Property2String(prop));
}

static void slot_register_helper_properties(int id, const PropertyList &props)
{
    std::cerr << "slot_register_helper_properties\n";
//     helper_props_map[id] = props;
    // TODO logo prop
    std::vector<String> list;
    PropertyList::const_iterator it = props.begin();
    PropertyList::const_iterator end = props.end();
    PropertyList::const_iterator next = it;
    while (it != end) {
        ++next;
        if (next == end || !next->is_a_leaf_of(*it)) {
            list.push_back(Property2String(*it));
            it = next;
        }
    }
    panel->RegisterProperties(list);
}

static void slot_update_helper_property(int id, const Property& prop)
{
    std::cerr << "slot_update_helper_property\n";
    //TODO
//     helper_props_map[id];
//     PropertyList::const_iterator it = props.begin();
//     PropertyList::const_iterator end = props.end();
//     PropertyList::const_iterator next = it;
//     while (it != end) {
//         ++next;
//         if (next == end || !next->is_a_leaf_of(*it)) {
//             panel->UpdateProperty(Property2String(*it));
//             it = next;
//         }
//     }
}

static void slot_register_helper(int id, const HelperInfo &helper)
{
    std::cerr << "slot_register_helper\n";
    //TODO
}

static void slot_remove_helper(int id)
{
    std::cerr << "slot_remove_helper\n";
    //TODO
//     helper_props_map[id]
}

static void slot_lock(void)
{
    std::cerr << "slot_lock\n";
    //_panel_agent_lock.lock();
}

static void slot_unlock(void)
{
    std::cerr << "slot_unlock\n";
    //_panel_agent_lock.unlock();
}

void niam(int sig)
{
    dispatcher.leave();
    delete panel;

    if (panel_agent) panel_agent->stop();
    delete panel_agent;
}

int main(int argc, char* argv[])
{
    String config_name("simple");
    String display_name;
    bool daemon = false;
    bool should_resident = true;

    //parse command options
    int i = 1;
    while (i < argc) {
        if (String("-l") == argv[i] || String("--list") == argv[i]) {
            std::cout << "\n";
            std::cout << "Available Config module:\n";
            // get config module list
            std::vector<String> config_list;
            scim_get_config_module_list(config_list);
            config_list.push_back("dummy");
            std::vector<String>::iterator it = config_list.begin();
            for (; it != config_list.end(); ++it) {
                std::cout << "    " << *it << "\n";
            }
            return 0;
        }
        else if (String("-c") == argv[i] || String("--config") == argv[i]) {
            if (++i >= argc) {
                std::cerr << "no argument for option " << argv[i-1] << "\n";
                return -1;
            }
            config_name = argv[i];
        }
        else if (String("-h") == argv[i] || String("--help") == argv[i]) {
            std::cout << "Usage: " << argv [0] << " [option]...\n\n"
                      << "The options are: \n"
                      << "  --display DISPLAY    Run on display DISPLAY.\n"
                      << "  -l, --list           List all of available config modules.\n"
                      << "  -c, --config NAME    Uses specified Config module.\n"
                      << "  -d, --daemon         Run " << argv [0] << " as a daemon.\n"
                      << "  -ns, --no-stay       Quit if no connected client.\n"
                      << "  -h, --help           Show this help message.\n";
            return 0;
        }
        else if (String("-d") == argv[i] || String("--daemon") == argv[i]) {
            daemon = true;
        }
        else if (String("-ns") == argv[i] || String("--no-stay") == argv[i]) {
            should_resident = false;
        }
        else if (String("--display") == argv[i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv[i-1] << "\n";
                return -1;
            }
            display_name = argv[i];
        }
        else {
            std::cerr << "Invalid command line option: " << argv[i] << "\n";
            return -1;
        }
        ++i;
    }

    // Make up DISPLAY env.
    if (display_name.length()) {
        setenv("DISPLAY", display_name.c_str(), 1);
    }

    if (config_name == "dummy") {
        _config = new DummyConfig();
    }
    else {
        _config_module = new ConfigModule(config_name);
        if (!_config_module || !_config_module->valid()) {
            std::cerr << "Can not load " << config_name << " Config module.\n";
            return -1;
        }
        _config = _config_module->create_config();
    }

    if (_config.null()) {
        std::cerr << "Failed to create instance from " << config_name << " Config module.\n";
        return -1;
    }


    signal(SIGTERM, niam);
    signal(SIGINT, niam);

    if (!initialize_panel_agent(config_name, display_name, should_resident)) {
        std::cerr << "Failed to initialize PanelAgent.\n";
        return -1;
    }

    if (daemon)
        scim_daemon();

    if (!run_panel_agent()) {
        std::cerr << "Failed to run Socket Server!\n";
        return -1;
    }

    start_auto_start_helpers();

    DBus::default_dispatcher = &dispatcher;

    DBus::Connection conn = DBus::Connection::SessionBus();
    conn.request_name("org.kde.impanel.inputmethod");

    panel = new Panel(conn);

    dispatcher.enter();

    return 0;
}
