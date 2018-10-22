#ifndef GUI_MORE_OPTIONS_H
#define GUI_MORE_OPTIONS_H

#include <QDialog>
#include <QMap>
#include <QString>

#include "gui-helper.h"

typedef struct MoreOptions_struct {
    bool reset_on_tab_switch;
    bool send_little_endian;
    uint32_t chunk_size;
    QMap<QString, QStringList> checksum_map;
} MoreOptions_struct;

namespace Ui {
class GUI_MORE_OPTIONS;
}

class GUI_MORE_OPTIONS : public QDialog
{
    Q_OBJECT

public:
    explicit GUI_MORE_OPTIONS(MoreOptions_struct* main_options, QStringList GUIs, QStringList checksums, QWidget *parent = 0);
    ~GUI_MORE_OPTIONS();

    void reset_gui();

private slots:
    void on_GUI_Combo_activated(int);
    void on_Checksum_Combo_activated(int);
    void on_ChecksumSet_Button_clicked();
    void on_BrowseEXE_Button_clicked();

    void on_Apply_Button_clicked();
    void on_Cancel_Button_clicked();
    void on_OK_Button_clicked();

private:
    Ui::GUI_MORE_OPTIONS *ui;
    bool updated;

    MoreOptions_struct* main_options_ptr;

    void save_updates();
};

#endif // GUI_MORE_OPTIONS_H
