#include "palettebutton.h"
#include <QDebug>

PaletteButton::PaletteButton(const QColor& color, QWidget* parent)
    : QPushButton(parent)
    , m_color(color)
{
    setText(color.name());
    setToolTip("Add this color to the canvas palette");
    qDebug() << "[CREATE] PaletteButton" << color.name();
}

PaletteButton::~PaletteButton()
{
    qDebug() << "[DESTROY] PaletteButton" << m_color.name();
}