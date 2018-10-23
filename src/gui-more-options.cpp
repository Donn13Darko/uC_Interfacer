#include "gui-more-options.h"
#include "ui_gui-more-options.h"

GUI_MORE_OPTIONS::GUI_MORE_OPTIONS(MoreOptions_struct* main_options, MoreOptions_struct** local_options_ptr, QStringList GUIs, QStringList checksums, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GUI_MORE_OPTIONS)
{
    // Setup ui
    ui->setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint
                   | Qt::MSWindowsFixedSizeDialogHint);

    // Link options
    main_options_ptr = main_options;
    *local_options_ptr = &local_options;

    // Enter combos
    ui->GUI_Combo->addItems(GUIs);
    ui->Checksum_Combo->addItems(checksums);

    // Reset GUI
    reset_gui();
}

GUI_MORE_OPTIONS::~GUI_MORE_OPTIONS()
{
    delete ui;
}

void GUI_MORE_OPTIONS::reset_gui()
{
    // Reset variables
    updated = false;
    reset_updates();

    // Reset fields
    ui->ResetOnTabSwitch_CheckBox->setChecked(local_options.reset_on_tab_switch);
    ui->SendLittleEndian_CheckBox->setChecked(local_options.send_little_endian);
    ui->ChunkSize_LineEdit->setText(QString::number(local_options.chunk_size));

    // Reset Checksum combos
    ui->GUI_Combo->setCurrentIndex(0);
    on_GUI_Combo_activated(0);
}

void GUI_MORE_OPTIONS::on_GUI_Combo_activated(int)
{
    // Updated checksum settings (set to current default)
    QString gui = ui->GUI_Combo->currentText();
    QStringList gui_checksum = local_options.checksum_map.value(
                gui, main_options_ptr->checksum_map.value(
                    gui, DEFAULT_CHECKSUM)
                );
    ui->Checksum_Combo->setCurrentText(gui_checksum.at(checksum_name_pos));

    // Update checksum
    on_Checksum_Combo_activated(0);
}

void GUI_MORE_OPTIONS::on_Checksum_Combo_activated(int)
{
    // Get current state info
    QString curr_gui_combo = ui->GUI_Combo->currentText();
    QString curr_check_combo = ui->Checksum_Combo->currentText();

    // Get checksum info if its stored (otherwise set to default)
    QStringList checksum_info = \
            local_options.checksum_map.value(
                curr_gui_combo,
                main_options_ptr->checksum_map.value(curr_gui_combo,
                                                     DEFAULT_CHECKSUM)
                );

    // Populate fields if current entry
    if (checksum_info.at(checksum_name_pos) == curr_gui_combo)
    {
        // Update exe path
        ui->ChecksumEXE_LineEdit->setText(checksum_info.at(checksum_exe_pos));

        // Update start value
        ui->ChecksumStart_LineEdit->setText(checksum_info.at(checksum_start_pos));
    }

    // Enable or disable EXE features
    bool isEXE = (curr_check_combo == "CHECKSUM_EXE");
    ui->ChecksumEXE_LineEdit->setEnabled(isEXE);
    ui->BrowseEXE_Button->setEnabled(isEXE);
}

void GUI_MORE_OPTIONS::on_ChecksumSet_Button_clicked()
{
    // Save checksum info
    QString gui = ui->GUI_Combo->currentText();
    QString checksum = ui->Checksum_Combo->currentText();
    QString exe = ui->ChecksumEXE_LineEdit->text();

    // If EXE, verify path input
    if ((checksum == "CHECKSUM_EXE")
            && exe.isEmpty())
    {
        GUI_HELPER::showMessage("Error: Checksum EXE path required!");
        return;
    }

    // Save to local checksum info
    local_options.checksum_map.insert(gui, {checksum,
                                           ui->ChecksumStart_LineEdit->text(),
                                           exe}
                                     );
}

void GUI_MORE_OPTIONS::on_BrowseEXE_Button_clicked()
{
    // Select file to send
    QString file;
    if (GUI_HELPER::getOpenFilePath(&file, tr("Executables (*.exe);; All Files (*.*)")))
        ui->ChecksumEXE_LineEdit->setText(file);
}

void GUI_MORE_OPTIONS::on_Undo_Button_clicked()
{
    // Reset gui but don't exit
    reset_gui();
}

void GUI_MORE_OPTIONS::on_Apply_Button_clicked()
{
    // Save data but don't exit
    save_updates();
}

void GUI_MORE_OPTIONS::on_Cancel_Button_clicked()
{
    // Accept or reject if dialog updated
    if (updated) accept();
    else reject();
}

void GUI_MORE_OPTIONS::on_OK_Button_clicked()
{
    // Save data
    save_updates();

    // Close dialog
    accept();
}

void GUI_MORE_OPTIONS::save_updates()
{
    // Overwrite members of main_options_ptr
    main_options_ptr->reset_on_tab_switch = ui->ResetOnTabSwitch_CheckBox->isChecked();
    main_options_ptr->send_little_endian = ui->SendLittleEndian_CheckBox->isChecked();
    main_options_ptr->chunk_size = ui->ChunkSize_LineEdit->text().toInt();

    // Save local checksum map
    foreach (QString key, local_options.checksum_map.keys())
    {
        main_options_ptr->checksum_map.insert(key, local_options.checksum_map.value(key));
    }

    // Mark as updated
    updated = true;
}

void GUI_MORE_OPTIONS::reset_updates()
{
    // Overwrite local options
    local_options.reset_on_tab_switch = main_options_ptr->reset_on_tab_switch;
    local_options.send_little_endian = main_options_ptr->send_little_endian;
    local_options.chunk_size = main_options_ptr->chunk_size;

    // Clear local checksum map
    local_options.checksum_map.clear();
}
