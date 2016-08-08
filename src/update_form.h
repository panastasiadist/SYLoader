#ifndef UPDATE_FORM_H
#define UPDATE_FORM_H

#include <QWidget>

namespace Ui {
class UpdateForm;
}

class UpdateForm : public QWidget
{
    Q_OBJECT

public:
    explicit UpdateForm(QWidget *parent = 0);
    ~UpdateForm();

private:
    Ui::UpdateForm *ui;
};

#endif // UPDATE_FORM_H
