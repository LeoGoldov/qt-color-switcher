#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ColorSwitcher.h"

class ColorSwitcher : public QMainWindow
{
    Q_OBJECT

public:
    ColorSwitcher(QWidget *parent = nullptr);
    ~ColorSwitcher();

private:
    Ui::ColorSwitcherClass ui;
};
