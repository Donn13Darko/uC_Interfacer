#include "gui-more-options.h"
#include "ui_gui-more-options.h"

GUI_MORE_OPTIONS::GUI_MORE_OPTIONS(MoreOptions_struct* main_options, QStringList GUIs, QStringList checksums, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GUI_MORE_OPTIONS)
{
    // Setup ui
    ui->setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint
                   | Qt::MSWindowsFixedSizeDialogHint);

    // Link options & combos
    main_options_ptr = main_options;
    ui->GUI_Combo->addItems(GUIs);
    ui->Checksum_Combo->addItems(checksums);

    // reset
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

    // Reset fields
    ui->ResetOnTabSwitch_CheckBox->setChecked(main_options_ptr->reset_on_tab_switch);
    ui->SendLittleEndian_CheckBox->setChecked(main_options_ptr->send_little_endian);
    ui->ChunkSize_LineEdit->setText(QString::number(main_options_ptr->chunk_size));

    // Reset Checksum combos
    ui->GUI_Combo->setCurrentIndex(0);
    on_GUI_Combo_activated(0);
}

void GUI_MORE_OPTIONS::on_GUI_Combo_activated(int)
{
    // Updated checksum settings (set to current default)
    QString gui = ui->GUI_Combo->currentText();
    QStringList gui_checksum = main_options_ptr->checksum_map.value(gui, {"CRC_8_LUT", ""});
    ui->Checksum_Combo->setCurrentText(gui_checksum.at(0));

    // Update checksum
    on_Checksum_Combo_activated(0);
}

void GUI_MORE_OPTIONS::on_Checksum_Combo_activated(int)
{
    // Check if EXE
    bool isEXE = (ui->Checksum_Combo->currentText() == "CHECKSUM_EXE");

    // Enable or disable EXE features
    ui->ChecksumEXE_LineEdit->setEnabled(isEXE);
    ui->BrowseEXE_Button->setEnabled(isEXE);

    // Clear EXE path edit
    if (isEXE)
    {
        ui->ChecksumEXE_LineEdit->setText(
                    main_options_ptr->checksum_map.value(
                        ui->GUI_Combo->currentText()).at(1)
                    );
    } else
    {
        ui->ChecksumEXE_LineEdit->clear();
    }
}

void GUI_MORE_OPTIONS::on_ChecksumSet_Button_clicked()
{
    // Save checksum info
    QString gui = ui->GUI_Combo->currentText();
    QString checksum = ui->Checksum_Combo->currentText();

    // If EXE, verify path input
    if ((checksum == "CHECKSUM_EXE")
            && ui->ChecksumEXE_LineEdit->text().isEmpty())
    {
        GUI_HELPER::showMessage("Error: EXE path required!");
        return;
    }

    // Save checksum info
    main_options_ptr->checksum_map.insert(gui, {checksum, ui->ChecksumEXE_LineEdit->text()});

    // Mark as updated
    updated = true;
}

void GUI_MORE_OPTIONS::on_BrowseEXE_Button_clicked()
{
    // Select file to send
    QString file;
    if (GUI_HELPER::getOpenFilePath(&file, tr("Executables (*.exe);; All Files (*.*)")))
        ui->ChecksumEXE_LineEdit->setText(file);
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

    // Mark as updated
    updated = true;
}
