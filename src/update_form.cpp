#include "update_form.h"
#include "ui_update_form.h"

UpdateForm::UpdateForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UpdateForm)
{
    ui->setupUi(this);
}

UpdateForm::~UpdateForm()
{
    delete ui;
}
