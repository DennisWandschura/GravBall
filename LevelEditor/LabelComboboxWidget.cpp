#include "LabelComboboxWidget.h"

LabelComboboxWidget::LabelComboboxWidget(QWidget* parent, const char* text)
	:QWidget(parent),
	m_label(nullptr),
	m_combobox(nullptr)
{
	resize(60 + 75 + 5, 25);
	m_label = new QLabel(this);
	m_label->setText(text);
	m_label->resize(60, 20);

	m_combobox = new QComboBox(this);
	m_combobox->resize(75, 20);
	m_combobox->move(60, 0);
}

void LabelComboboxWidget::addItem(const QString &text)
{
	m_combobox->addItem(text);
}

void LabelComboboxWidget::setSelected(int i)
{
	m_combobox->setCurrentIndex(i);
}

int LabelComboboxWidget::getSelectedIndex() const
{
	return m_combobox->currentIndex();
}

const QString& LabelComboboxWidget::getSelectedText() const
{
	return m_combobox->currentText();
}