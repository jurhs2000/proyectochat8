/**
 * @file client.c
 * @author Julio H., Oscar S.
 * @brief 
 * @version 0.1
 * 
 * Universidad del Valle de Guatemala
 * Sistemas Operativos - CC3064
 * Proyecto - Chat
 */
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <gtk/gtk.h>
#include <time.h>
#include <cjson/cJSON.h>

int socket_desc;
char *user;
GtkWidget *window;
GtkWidget *grid;

static void show_message(char error[]) {
    GtkWidget *dialog;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL;
    dialog = gtk_message_dialog_new (NULL, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                    "Error: %s",
                                    error);
    g_signal_connect (dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
    gtk_widget_show(dialog);
}


static int send_message(GtkWidget *widget, GtkEntryBuffer *buffer)
{
    const gchar *entry_text;
    entry_text = gtk_entry_buffer_get_text(buffer); //Send init connection data
    // Creating JSON object
    cJSON *post_chat = cJSON_CreateObject();
    cJSON *request = NULL;
    cJSON *body = NULL;
    cJSON *message = NULL;
    cJSON *from = NULL;
    cJSON *delivered_at = NULL;
    cJSON *to = NULL;
    request = cJSON_CreateString("POST_CHAT");
    cJSON_AddItemToObject(post_chat, "request", request);
    body = cJSON_CreateArray();
    time_t seconds;
    struct tm *timeinfo;
    time(&seconds);
    timeinfo = localtime(&seconds);
    delivered_at = cJSON_CreateString(asctime(timeinfo));
    from = cJSON_CreateString(user);
    message = cJSON_CreateString(entry_text);
    to = cJSON_CreateString("all");
    cJSON_AddItemToArray(body, delivered_at);
    cJSON_AddItemToArray(body, from);
    cJSON_AddItemToArray(body, message);
    cJSON_AddItemToArray(body, to);
    cJSON_AddItemToObject(post_chat, "body", body);
    // Parsing JSON to string
    char *post_chat_str;
    post_chat_str = cJSON_Print(post_chat);
    // Sending connection request
	if( send(socket_desc , post_chat_str , strlen(post_chat_str) , 0) < 0)
	{
		puts("Message can not be sended");
		return 1;
	}
	puts("Message sended successfully\n");
    // Clean buffer
    gtk_entry_buffer_set_text(buffer, "", 0);
}

static void get_chat(char *chat_str) {
    GtkWidget *scrolled, *view;
    scrolled = gtk_scrolled_window_new();
    view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 10);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(view), 10);
    GtkTextBuffer *text_buffer = gtk_text_buffer_new(NULL);
    GtkTextIter iter;
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), text_buffer);
    gtk_text_buffer_get_iter_at_offset(text_buffer, &iter, 0);
    const cJSON *messages = NULL;
    const cJSON *message = NULL;
    cJSON *chats_object = cJSON_Parse(chat_str);
    messages = cJSON_GetObjectItemCaseSensitive(chats_object, "body");
    char *property_string = NULL;
    char *message_formatted = (char *) malloc(8);
    strcpy(message_formatted, "");
    cJSON_ArrayForEach(message, messages) {
        message_formatted = (char *) realloc(message_formatted, strlen(message_formatted) + 512);
        cJSON *user = cJSON_GetArrayItem(message, 1);
        property_string = cJSON_Print(user);
        strcat(message_formatted, property_string);
        cJSON *text = cJSON_GetArrayItem(message, 0);
        property_string = cJSON_Print(text);
        strcat(message_formatted, property_string);
        cJSON *delivTime = cJSON_GetArrayItem(message, 2);
        property_string = cJSON_Print(delivTime);
        strcat(message_formatted, property_string);
        strcat(message_formatted, "\n");
    }
    gtk_text_buffer_insert(text_buffer, &iter, message_formatted, -1);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), view);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled), 600);
    gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(scrolled), 800);
    gtk_grid_attach(GTK_GRID(grid), scrolled, 0,2,3,1);
}

static void render_general_chat(char* chat_json) {
    gtk_grid_remove_row(GTK_GRID (grid), 2);
    gtk_grid_remove_row(GTK_GRID (grid), 1);
    GtkWidget *message_entry, *send_button;
    GtkEntryBuffer *buffer = gtk_entry_buffer_new("", 0);
    message_entry = gtk_entry_new_with_buffer (buffer);
    gtk_widget_set_margin_start(message_entry, 10);
    gtk_widget_set_margin_top(message_entry, 10);
    gtk_widget_set_margin_bottom(message_entry, 10);
    g_signal_connect(message_entry, "activate", G_CALLBACK(send_message), buffer);
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Ingresa un mensaje:");
    gtk_grid_attach (GTK_GRID (grid), message_entry, 0, 1, 2, 1);
    send_button = gtk_button_new_with_label ("Send Message");
    gtk_widget_set_margin_start(send_button, 10);
    gtk_widget_set_margin_end(send_button, 10);
    gtk_widget_set_margin_top(send_button, 10);
    gtk_widget_set_margin_bottom(send_button, 10);
    g_signal_connect (send_button, "clicked", G_CALLBACK (send_message), buffer);
    gtk_grid_attach (GTK_GRID (grid), send_button, 2, 1, 1, 1);
    char *chat_str = "{\"body\": [[\"mensaje 1\", \"user1\", \"18:03\"],[\"mensaje 2\", \"user2\", \"18:05\"],[\"mensaje 3\", \"user3\", \"19:21\"]]}";
    get_chat(chat_str);
}

static void receive_from_server() {
    //Receive a reply from the server
    char *server_reply = malloc(2000);
	if( recv(socket_desc, server_reply , 2000 , 0) < 0)
	{
		puts("recv failed");
	}
	puts("Reply received\n");
    puts(server_reply);
    show_message(server_reply);
    // render general chat if response is of type GET_CHAT
    render_general_chat(server_reply);
}

static void show_user_info(GtkWidget *widget, gpointer data)
{
    show_message("show user info");
}

static void show_users(GtkWidget *widget) {
    gtk_grid_remove_row(GTK_GRID (grid), 2);
    gtk_grid_remove_row(GTK_GRID (grid), 1);
    int users_size = 100;
    char user_text[50];
    GtkWidget *scrolled, *users_grid, *label, *info_button, *chat_button;
    scrolled = gtk_scrolled_window_new();
    users_grid = gtk_grid_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), users_grid);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled), 600);
    gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(scrolled), 800);
    gtk_grid_attach(GTK_GRID(grid), scrolled, 0,2,3,1);
    for (int i=0; i < users_size; ++i) {
        snprintf(user_text, 50, "Este es el usuario %d", i);
        label = gtk_label_new(user_text);
        gtk_widget_set_margin_start(label, 10);
        gtk_widget_set_margin_top(label, 10);
        gtk_grid_attach (GTK_GRID (users_grid), label, 0, i, 2, 1);
        info_button = gtk_button_new_with_label ("User Info");
        gtk_widget_set_margin_start(info_button, 10);
        gtk_widget_set_margin_top(info_button, 10);
        g_signal_connect (info_button, "clicked", G_CALLBACK (show_user_info), NULL);
        gtk_grid_attach (GTK_GRID (users_grid), info_button, 3, i, 1, 1);
        chat_button = gtk_button_new_with_label ("Chat User");
        gtk_widget_set_margin_start(chat_button, 10);
        gtk_widget_set_margin_top(chat_button, 10);
        g_signal_connect (chat_button, "clicked", G_CALLBACK (show_user_info), NULL);
        gtk_grid_attach (GTK_GRID (users_grid), chat_button, 4, i, 1, 1);
    }
}

static int create_connection(char *argv[]) {
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0)
    {
        printf("\n Error: Can not create the socket \n");
        return -1;
    }
    // Creating server
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[3]));
    server.sin_addr.s_addr = inet_addr(argv[2]);
    // Connecting to server
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("\n Error: Can not connect to server \n");
        return 1;
    }
    puts("Connected\n");
	//Send init connection data
    char *connection_init;
    cJSON *init_conex = cJSON_CreateObject();
    cJSON *request = NULL;
    cJSON *body = NULL;
    cJSON *connect_time = NULL;
    cJSON *user_id = NULL;
    request = cJSON_CreateString("INIT_CONEX");
    cJSON_AddItemToObject(init_conex, "request", request);
    body = cJSON_CreateArray();
    time_t seconds;
    struct tm *timeinfo;
    time(&seconds);
    timeinfo = localtime(&seconds);
    connect_time = cJSON_CreateString(asctime(timeinfo));
    user_id = cJSON_CreateString(user);
    cJSON_AddItemToArray(body, connect_time);
    cJSON_AddItemToArray(body, user_id);
    cJSON_AddItemToObject(init_conex, "body", body);
    connection_init = cJSON_Print(init_conex);
    // Sending connection request
	if( send(socket_desc , connection_init , strlen(connection_init) , 0) < 0)
	{
		puts("Init connection failed");
		return 1;
	}
	puts("Successful init connection\n");
}

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *show_users_button, *switch_status_button;

    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Window");
    gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

    grid = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(window), grid);

    show_users_button = gtk_button_new_with_label ("Show Users");
    gtk_widget_set_margin_start(show_users_button, 10);
    gtk_widget_set_margin_top(show_users_button, 10);
    gtk_widget_set_margin_bottom(show_users_button, 10);
    g_signal_connect (show_users_button, "clicked", G_CALLBACK (show_users), NULL);
    gtk_grid_attach (GTK_GRID (grid), show_users_button, 0, 0, 1, 1);

    switch_status_button = gtk_button_new_with_label ("Switch Status");
    gtk_widget_set_margin_start(switch_status_button, 10);
    gtk_widget_set_margin_top(switch_status_button, 10);
    gtk_widget_set_margin_bottom(switch_status_button, 10);
    g_signal_connect (switch_status_button, "clicked", G_CALLBACK (render_general_chat), NULL);
    gtk_grid_attach (GTK_GRID (grid), switch_status_button, 1, 0, 1, 1);

    gtk_window_present (GTK_WINDOW (window));

    puts("Listening to server\n");
    receive_from_server();
}

int main(int argc, char *argv[])
{
    user = argv[1];
    create_connection(argv);
    // Displaying window with GTK
    GtkApplication *app;
    int status;
    app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run (G_APPLICATION (app), 0, NULL);
    return status;
}
