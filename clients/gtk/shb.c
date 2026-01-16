#include <gtk/gtk.h>
#include "websocket_client.h"
struct app_config config;
GtkWidget *window;
GtkWidget *buttons[APP_SWITCH_COUNT];
const char *g_server_ip = "192.168.100.2"; // Default IP address
// CSS to define styles for classes
const char *css_data =
    "button.active { background: rgb(76, 175, 80); color: white; }"
    "button.active:hover { background: rgb(76, 200, 80); color: unset; }"
    "button.active:active { background: rgb(70, 140, 70); color: unset; }";

void ws_send_toggle_button(int i){
    char msg[] = {WS_MSG_ID_ACTUATOR, 0};
    int state = !((config.switch_values & (1 << i)) > 0);
    msg[1] = i << 4 | state & 0b1111;
    websocket_send_binary(msg, 2);
}

// Button click callback function
void on_button_clicked(GtkWidget *widget, gpointer data)
{
    int i = GPOINTER_TO_INT(data);
    g_print("You clicked %i!\n", i);
    ws_send_toggle_button(i);
}

// Function to update the clock label
gboolean update_clock(GtkLabel *clock_label)
{
    char buffer[64];
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);

    // Format the current time as HH:MM:SS
    strftime(buffer, sizeof(buffer), "%H:%M:%S", local_time);

    // Set the updated time to the clock label
    gtk_label_set_text(clock_label, buffer);

    // Return TRUE to keep the timer running
    return TRUE;
}
void on_message(const char *message)
{
    printf("Received: %s\n", message);
}
void config_print()
{
    printf("App Config:\n"
           "\tconfig_version: %u\n"
           "\tswitch_len: %u\n"
           "\tswitch_values: %u\n"
           "\tsensor_len: %u\n"
           "\tsensor_delay: %u\n"
           "\thostname: %s\n",
           config.config_version,
           config.switch_len,
           config.switch_values,
           config.sensor_len,
           config.sensor_delay,
           config.hostname);
}


void update_button_state()
{
    int state;
    for (size_t i = 0; i < APP_SWITCH_COUNT; i++)
    {
        if( !(config.switch_cfg[i] & (0x1 << 7)) ){
            // Check disabled
            continue; 
        }

        state = (config.switch_values & (1 << i)) > 0;
            // printf("Button %lu status %d\n", i, state);
        if( state ){
            gtk_widget_add_css_class(buttons[i], "active");
        }else{
            gtk_widget_remove_css_class(buttons[i], "active");
        }
    }
}
void on_ws_config(struct lws *client, const unsigned char *msg, size_t size)
{
    memcpy(&config, msg, sizeof(struct app_config));
    // config_print();
    if ( strlen(config.hostname) )
    {
        gtk_window_set_title(GTK_WINDOW(window), config.hostname);
    }

    for (size_t i = 0; i < APP_SWITCH_COUNT; i++)
    {
        // snprintf("%s")
        gtk_button_set_label(GTK_BUTTON(buttons[i]), (const char *) &config.switches[i]);
        gtk_widget_set_sensitive(buttons[i], (int)(config.switch_cfg[i] & (0x1 << 7)));
    }
    update_button_state();
}
void on_ws_actuator(struct lws *client, const unsigned char *msg, size_t size)
{
    config.switch_values = msg[0];
    // config_print();
    update_button_state();
}
// Application activate callback
void on_app_activate(GApplication *app, gpointer user_data)
{
    // Create a CSS provider and load the CSS data
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css_data, -1);

    // Add the CSS provider to the default display
    GdkDisplay *display = gdk_display_get_default();
    gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Create the main application window
    window = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(window), "Conecting..");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Disable auto-focus on the first button
    gtk_window_set_focus_visible(GTK_WINDOW(window), FALSE);

    // Create a grid layout for buttons
    GtkWidget *grid = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(window), grid);

    // Make the grid expand to fill the entire window
    gtk_widget_set_hexpand(grid, TRUE);
    gtk_widget_set_vexpand(grid, TRUE);

    // Create a clock label
    GtkWidget *clock_label = gtk_label_new("..");
    gtk_grid_attach(GTK_GRID(grid), clock_label, 0, 0, 2, 1);
    gtk_widget_set_hexpand(clock_label, TRUE);
    gtk_widget_set_vexpand(clock_label, TRUE);
    // Start the clock update timer
    g_timeout_add_seconds(1, (GSourceFunc)update_clock, clock_label);

    for (size_t i = 0; i < APP_SWITCH_COUNT; i++)
    {
        GtkWidget *btn = gtk_button_new_with_label("...");
        buttons[i] = btn;
        gtk_grid_attach(GTK_GRID(grid), btn, i % 2, 1 + i / 2, 1, 1);
        gtk_widget_set_hexpand(btn, TRUE);
        gtk_widget_set_vexpand(btn, TRUE);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_button_clicked), GINT_TO_POINTER(i));
    }
    // Show the window
    gtk_window_present(GTK_WINDOW(window));
    // Release CSS provider (not strictly necessary as it will persist)
    g_object_unref(provider);

    web_socket_add_handler(WS_MSG_ID_CONFIG, &on_ws_config);
    web_socket_add_handler(WS_MSG_ID_ACTUATOR, &on_ws_actuator);
    // Set the message callback
    websocket_set_on_message(on_message);
    // Initialize the WebSocket client
    websocket_client_init(g_server_ip, "lws-minimal");
    // Start the WebSocket client in a separate thread
    websocket_client_start_threaded();
}


int main(int argc, char *argv[])
{
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ip") == 0 && i + 1 < argc) {
            g_server_ip = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("Options:\n");
            printf("  --ip ADDRESS    Set server IP address (default: 192.168.100.2)\n");
            printf("  --help          Show this help message\n");
            return 0;
        }
    }
    
    gtk_init();

    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-font-name", "Arial 20", NULL);
    g_object_set(settings, "gtk-theme-name", "Adwaita-dark", NULL);

    // Create a new GTK application
    GtkApplication *app = gtk_application_new("com.osh.switches", G_APPLICATION_DEFAULT_FLAGS);

    // Connect the activate signal to set up the UI
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    

    // Run the application (this starts the main loop internally)
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    // Cleanup
    g_object_unref(app);
    // Stop the WebSocket client
    websocket_client_stop();

    return status;
}
