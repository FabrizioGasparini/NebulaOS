#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct calculator {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *display;
    char expression[256];
    int expr_len;
    double result;
    gboolean last_was_result;
};

static void update_display(struct calculator *calc) {
    gtk_editable_set_text(GTK_EDITABLE(calc->display), calc->expression);
}

static void clear(struct calculator *calc) {
    calc->expr_len = 0;
    calc->expression[0] = '\0';
    calc->last_was_result = FALSE;
    update_display(calc);
}

static void append_char(struct calculator *calc, const char *ch) {
    if (calc->last_was_result) {
        clear(calc);
    }
    size_t ch_len = strlen(ch);
    if (calc->expr_len + (int)ch_len < (int)sizeof(calc->expression)) {
        strcat(calc->expression, ch);
        calc->expr_len += ch_len;
        update_display(calc);
    }
}

static int calc_precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

static double calc_apply(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return b != 0 ? a / b : 0;
    }
    return 0;
}

static void calculate(struct calculator *calc) {
    char expr[256];
    snprintf(expr, sizeof(expr), "%s", calc->expression);

    char *p = expr;
    double values[64];
    char ops[64];
    int val_top = -1, op_top = -1;

    while (*p) {
        if (*p == ' ') { p++; continue; }
        if (isdigit((unsigned char)*p) || (*p == '.')) {
            char *end;
            double val = strtod(p, &end);
            values[++val_top] = val;
            p = end;
        } else if (*p == '(') {
            ops[++op_top] = *p;
            p++;
        } else if (*p == ')') {
            while (op_top >= 0 && ops[op_top] != '(') {
                double b = values[val_top--];
                double a = values[val_top--];
                values[++val_top] = calc_apply(a, b, ops[op_top--]);
            }
            if (op_top >= 0) op_top--;
            p++;
        } else {
            while (op_top >= 0 && ops[op_top] != '(' &&
                   calc_precedence(ops[op_top]) >= calc_precedence(*p)) {
                double b = values[val_top--];
                double a = values[val_top--];
                values[++val_top] = calc_apply(a, b, ops[op_top--]);
            }
            ops[++op_top] = *p;
            p++;
        }
    }

    while (op_top >= 0) {
        double b = values[val_top--];
        double a = values[val_top--];
        values[++val_top] = calc_apply(a, b, ops[op_top--]);
    }

    calc->result = values[val_top];
    snprintf(calc->expression, sizeof(calc->expression), "%g", calc->result);
    calc->expr_len = strlen(calc->expression);
    calc->last_was_result = TRUE;
    update_display(calc);
}

static void on_button_clicked(GtkButton *button, gpointer user_data) {
    struct calculator *calc = user_data;
    const char *label = gtk_button_get_label(button);

    if (strcmp(label, "C") == 0) {
        clear(calc);
    } else if (strcmp(label, "=") == 0) {
        calculate(calc);
    } else if (strcmp(label, "\u232b") == 0) {
        if (calc->expr_len > 0) {
            calc->expression[--calc->expr_len] = '\0';
            update_display(calc);
        }
    } else {
        append_char(calc, label);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    struct calculator *calc = user_data;

    calc->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(calc->window), "NebulaOS Calculator");
    gtk_window_set_resizable(GTK_WINDOW(calc->window), FALSE);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_margin_start(main_box, 8);
    gtk_widget_set_margin_end(main_box, 8);
    gtk_widget_set_margin_top(main_box, 8);
    gtk_widget_set_margin_bottom(main_box, 8);
    gtk_window_set_child(GTK_WINDOW(calc->window), main_box);

    calc->display = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(calc->display), FALSE);
    gtk_widget_set_halign(calc->display, GTK_ALIGN_FILL);
    gtk_widget_add_css_class(calc->display, "calculator-display");
    gtk_box_append(GTK_BOX(main_box), calc->display);

    const char *labels[][4] = {
        {"C", "\u232b", "/", "*"},
        {"7", "8", "9", "-"},
        {"4", "5", "6", "+"},
        {"1", "2", "3", "="},
        {"0", ".", "(", ")"},
    };

    for (int row = 0; row < 5; row++) {
        GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        gtk_box_append(GTK_BOX(main_box), button_box);

        for (int col = 0; col < 4; col++) {
            GtkWidget *btn = gtk_button_new_with_label(labels[row][col]);
            gtk_widget_add_css_class(btn, "calculator-button");

            if (col == 3) gtk_widget_add_css_class(btn, "calc-op");
            else if (row == 0) gtk_widget_add_css_class(btn, "calc-fn");
            else gtk_widget_add_css_class(btn, "calc-num");

            if (strcmp(labels[row][col], "=") == 0)
                gtk_widget_add_css_class(btn, "calc-eq");

            g_signal_connect(btn, "clicked", G_CALLBACK(on_button_clicked), calc);
            gtk_box_append(GTK_BOX(button_box), btn);
        }
    }

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        ".calculator-display {"
        "  background-color: rgba(10, 3, 22, 0.6);"
        "  border: 1px solid rgba(139, 92, 246, 0.3);"
        "  border-radius: 8px; padding: 12px;"
        "  font-size: 24px; font-weight: bold;"
        "  color: rgba(255, 255, 255, 0.95); margin-bottom: 8px; }"
        ".calculator-button { min-height: 48px; border-radius: 8px; font-size: 18px; }"
        ".calc-num { background-color: rgba(30, 15, 55, 0.6); border: 1px solid rgba(139,92,246,0.2); color: white; }"
        ".calc-num:hover { background-color: rgba(139, 92, 246, 0.2); }"
        ".calc-op { background-color: rgba(139, 92, 246, 0.3); border: 1px solid rgba(139,92,246,0.4); color: white; }"
        ".calc-op:hover { background-color: rgba(139, 92, 246, 0.45); }"
        ".calc-fn { background-color: rgba(20, 10, 40, 0.5); border: 1px solid rgba(255,255,255,0.1); color: rgba(221,214,254,0.8); }"
        ".calc-fn:hover { background-color: rgba(139, 92, 246, 0.2); }"
        ".calc-eq { background-color: rgba(139, 92, 246, 0.6); border: 1px solid rgba(139,92,246,0.7); color: white; }"
        ".calc-eq:hover { background-color: rgba(139, 92, 246, 0.8); }");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_window_present(GTK_WINDOW(calc->window));
}

int main(int argc, char *argv[]) {
    struct calculator calc = {0};
    calc.app = gtk_application_new("com.nebulaos.calculator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(calc.app, "activate", G_CALLBACK(activate), &calc);
    calc.expression[0] = '\0';
    calc.expr_len = 0;
    int status = g_application_run(G_APPLICATION(calc.app), argc, argv);
    g_object_unref(calc.app);
    return status;
}
