#include "LabelSpinboxWidget.h"

LabelSpinboxWidget::LabelSpinboxWidget(QWidget* parent, const char* text)
	:QWidget(parent),
	m_label(nullptr),
	m_spinbox(nullptr)
{
	resize(60 + 75 + 5, 25);
	m_label = new QLabel(this);
	m_label->setText(text);
	m_label->resize(60, 20);

	m_spinbox = new QDoubleSpinBox(this);

	m_spinbox->resize(75, 20);

	m_spinbox->move(60, 0);

	m_spinbox->setMaximum(9999.0);
	m_spinbox->setMinimum(-9999.0);
}

void LabelSpinboxWidget::setMinMax(float min, float max)
{
	m_spinbox->setMaximum(max);
	m_spinbox->setMinimum(min);
}

void LabelSpinboxWidget::setValue(float v)
{
	m_spinbox->setValue(v);
}

float LabelSpinboxWidget::getValue() const
{
	return m_spinbox->value();
}