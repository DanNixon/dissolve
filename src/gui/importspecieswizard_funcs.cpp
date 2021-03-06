/*
	*** Add Species Wizard Functions
	*** src/gui/importspecieswizard_funcs.cpp
	Copyright T. Youngs 2012-2020

	This file is part of Dissolve.

	Dissolve is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Dissolve is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Dissolve.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/importspecieswizard.h"
#include "gui/helpers/listwidgetupdater.h"
#include "gui/helpers/treewidgetupdater.h"
#include "main/dissolve.h"
#include "classes/atomtype.h"
#include "classes/species.h"
#include "templates/variantpointer.h"
#include <QFileDialog>
#include <QInputDialog>

// Constructor / Destructor
ImportSpeciesWizard::ImportSpeciesWizard(QWidget* parent) : temporaryDissolve_(temporaryCoreData_)
{
	dissolveReference_ = NULL;
	importTarget_ = NULL;

	// Set up our UI, and attach the wizard's widgets to placeholder widgets (if available)
	ui_.setupUi(this);
	setUpHeaderAndFooter(this);

	// Create parent items for MasterTerms tree
	masterBondItemParent_ = new QTreeWidgetItem(ui_.MasterTermsTree);
	masterBondItemParent_->setFlags(Qt::ItemIsEnabled);
	masterBondItemParent_->setExpanded(true);
	masterBondItemParent_->setText(0, "Bonds");
	masterAngleItemParent_ = new QTreeWidgetItem(ui_.MasterTermsTree);
	masterAngleItemParent_->setFlags(Qt::ItemIsEnabled);
	masterAngleItemParent_->setText(0, "Angles");
	masterAngleItemParent_->setExpanded(true);
	masterTorsionItemParent_ = new QTreeWidgetItem(ui_.MasterTermsTree);
	masterTorsionItemParent_->setFlags(Qt::ItemIsEnabled);
	masterTorsionItemParent_->setText(0, "Torsions");
	masterTorsionItemParent_->setExpanded(true);

	// Register pages with the wizard
	registerPage(ImportSpeciesWizard::SelectFilePage, "Choose Input File", ImportSpeciesWizard::SelectSpeciesPage);
	registerPage(ImportSpeciesWizard::SelectSpeciesPage, "Select Species", ImportSpeciesWizard::AtomTypesPage);
	registerPage(ImportSpeciesWizard::AtomTypesPage, "Atom Types");
	registerPage(ImportSpeciesWizard::MasterTermsPage, "Master Terms", ImportSpeciesWizard::SpeciesNamePage);
	registerPage(ImportSpeciesWizard::SpeciesNamePage, "Species Name", WizardWidgetPageInfo::FinishHereFlag);

	// Connect signals / slots
	connect(ui_.AtomTypesList->itemDelegate(), SIGNAL(commitData(QWidget*)), this, SLOT(atomTypesListEdited(QWidget*)));
	connect(ui_.MasterTermsTree->itemDelegate(), SIGNAL(commitData(QWidget*)), this, SLOT(masterTermsTreeEdited(QWidget*)));

	lockedForRefresh_ = 0;
}

ImportSpeciesWizard::~ImportSpeciesWizard()
{
}

/*
 * Data
 */

// Set Dissolve reference
void ImportSpeciesWizard::setMainDissolveReference(const Dissolve* dissolveReference)
{
	dissolveReference_ = dissolveReference;
}

// Copy imported Species over to the specified Dissolve object
Species* ImportSpeciesWizard::importSpecies(Dissolve& dissolve)
{
	// Check that we have a target Species (in temporaryDissolve_) to import
	if (!importTarget_)
	{
		Messenger::error("No target Species to import!\n");
		return NULL;
	}

	// Set the final name of the new Species
	importTarget_->setName(qPrintable(ui_.SpeciesNameEdit->text()));

	// Copy the importTarget_ over
	Species* newSpecies = dissolve.copySpecies(importTarget_);

	return newSpecies;
}

/*
 * Controls
 */

// Go to specified page index in the controls widget
bool ImportSpeciesWizard::displayControlPage(int index)
{
	// Check page index given
	if ((index < 0) || (index >= ImportSpeciesWizard::nPages)) return Messenger::error("Page index %i not recognised.\n", index);

	// Page index is valid, so show it - no need to switch/case
	ui_.MainStack->setCurrentIndex(index);

	// Update controls in the target page if necessary
	switch (index)
	{
		default:
			break;
	}

	return true;
}

// Return whether progression to the next page from the current page is allowed
bool ImportSpeciesWizard::progressionAllowed(int index) const
{
	// Check widget validity in the specified page, returning if progression (i.e. pushing 'Next' or 'Finish') is allowed
	switch (index)
	{
		case (ImportSpeciesWizard::SelectFilePage):
			return ((!ui_.InputFileEdit->text().isEmpty()) && (QFile::exists(ui_.InputFileEdit->text())));
		case (ImportSpeciesWizard::SelectSpeciesPage):
			return (ui_.SpeciesList->currentRow() != -1);
		case (ImportSpeciesWizard::SpeciesNamePage):
			return (ui_.SpeciesNameIndicator->state() == CheckIndicator::OKState);
		default:
			break;
	}

	return true;
}

// Perform any necessary actions before moving to the next page
bool ImportSpeciesWizard::prepareForNextPage(int currentIndex)
{
	SpeciesAtom* atomicAtom;

	switch (currentIndex)
	{
		case (ImportSpeciesWizard::SelectFilePage):
			// Check that the input/species file exists, and can be read in successfully
			if (!temporaryDissolve_.loadInput(qPrintable(ui_.InputFileEdit->text())))
			{
				return false;
			}
			// Update the Species and AtomTypes lists
			ui_.SpeciesList->clear();
			for (Species* sp = temporaryDissolve_.species().first(); sp != NULL; sp = sp->next())
			{
				QListWidgetItem* item = new QListWidgetItem(sp->name());
				item->setData(Qt::UserRole, VariantPointer<Species>(sp));
				ui_.SpeciesList->addItem(item);
			}
			if (ui_.SpeciesList->count() > 0)
			{
				ui_.SpeciesList->setCurrentRow(0);
				importTarget_ = temporaryCoreData_.species().first();
			}

			updateAtomTypesPage();
			updateMasterTermsPage();
			break;
		default:
			break;
	}

	return true;
}

// Determine next page for the current page, based on current data
int ImportSpeciesWizard::determineNextPage(int currentIndex)
{
	switch (currentIndex)
	{
		case (ImportSpeciesWizard::AtomTypesPage):
			// If there are master terms present, go to that page first. Otherwise, skip straight to naming
			if (temporaryCoreData_.nMasterBonds() || temporaryCoreData_.nMasterAngles() || temporaryCoreData_.nMasterTorsions()) return ImportSpeciesWizard::MasterTermsPage;
			else return ImportSpeciesWizard::SpeciesNamePage;
		default:
			break;
	}

	return -1;
}

// Perform any necessary actions before moving to the previous page
bool ImportSpeciesWizard::prepareForPreviousPage(int currentIndex)
{
	switch (currentIndex)
	{
		case (ImportSpeciesWizard::SelectSpeciesPage):
			// Clear the temporary dissolve instance
			temporaryDissolve_.clear();
		default:
			break;
	}

	return true;
}

/*
 * Controls
 */

// Reset widget, ready for adding a new Species
void ImportSpeciesWizard::reset()
{
	// Reset the underlying WizardWidget
	resetToPage(ImportSpeciesWizard::SelectFilePage);

	// Clear our temporary Dissolve instance
	temporaryDissolve_.clear();

	// Set a new, unique name ready on the final page
	ui_.SpeciesNameEdit->setText(dissolveReference_->constCoreData().uniqueSpeciesName("NewSpecies"));
}

/*
 * Import Species Pages
 */

void ImportSpeciesWizard::on_InputFileEdit_textChanged(const QString text)
{
	updateProgressionControls();
}

void ImportSpeciesWizard::on_InputFileSelectButton_clicked(bool checked)
{
	QString inputFile = QFileDialog::getOpenFileName(this, "Choose species / input file to open", QDir().absolutePath(), "Dissolve input / species files (*.txt *.dsp)");
	if (inputFile.isEmpty()) return;

	// Set input file in edit
	ui_.InputFileEdit->setText(inputFile);

	updateProgressionControls();
}

void ImportSpeciesWizard::on_SpeciesList_currentRowChanged(int currentRow)
{
	// Set import target from current row
	if (currentRow == -1) importTarget_ = NULL;
	else
	{
		importTarget_ = VariantPointer<Species>(ui_.SpeciesList->currentItem()->data(Qt::UserRole));
		ui_.SpeciesNameEdit->setText(importTarget_->name());
	}

	updateProgressionControls();
}

/*
 * AtomTypes Page
 */

// Row update function for AtomTypesList
void ImportSpeciesWizard::updateAtomTypesListRow(int row, AtomType* atomType, bool createItem)
{
	QListWidgetItem* item;
	if (createItem)
	{
		item = new QListWidgetItem;
		item->setData(Qt::UserRole, VariantPointer<AtomType>(atomType));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		ui_.AtomTypesList->insertItem(row, item);
	}
	else item = ui_.AtomTypesList->item(row);

	// Set item data
	item->setText(atomType->name());
	item->setIcon(QIcon(dissolveReference_->findAtomType(atomType->name()) ?  ":/general/icons/general_warn.svg" : ":/general/icons/general_true.svg"));
}

// Update page with AtomTypes in our temporary Dissolve reference
void ImportSpeciesWizard::updateAtomTypesPage()
{
	// Update the list against the global AtomType list
	ListWidgetUpdater<ImportSpeciesWizard,AtomType> listUpdater(ui_.AtomTypesList, temporaryCoreData_.constAtomTypes(), this, &ImportSpeciesWizard::updateAtomTypesListRow);

	// Determine whether we have any naming conflicts
	bool conflicts = false;
	ListIterator<AtomType> typeIterator(temporaryCoreData_.constAtomTypes());
	while (AtomType* at = typeIterator.iterate()) if (dissolveReference_->findAtomType(at->name()))
	{
		conflicts = true;
		break;
	}
	ui_.AtomTypesIndicator->setNotOK(conflicts);
	if (conflicts) ui_.AtomTypesIndicatorLabel->setText("One or more AtomTypes in the imported Species conflict with existing types");
	else ui_.AtomTypesIndicatorLabel->setText("There are no naming conflicts with the imported AtomTypes");
}

void ImportSpeciesWizard::on_AtomTypesList_itemSelectionChanged()
{
	// Enable / disable prefix and suffix buttons as appropriate
	bool isSelection = ui_.AtomTypesList->selectedItems().count() > 0;
	ui_.AtomTypesPrefixButton->setEnabled(isSelection);
	ui_.AtomTypesSuffixButton->setEnabled(isSelection);
}

void ImportSpeciesWizard::atomTypesListEdited(QWidget* lineEdit)
{
	// Since the signal that leads us here does not tell us the item that was edited, update all AtomType names here before updating the page
	for (int n=0; n<ui_.AtomTypesList->count(); ++n)
	{
		QListWidgetItem* item = ui_.AtomTypesList->item(n);
		AtomType* at = VariantPointer<AtomType>(item->data(Qt::UserRole));
		if (!at) continue;

		at->setName(qPrintable(item->text()));
	}

	updateAtomTypesPage();
}

void ImportSpeciesWizard::on_AtomTypesPrefixButton_clicked(bool checked)
{
	bool ok;
	QString prefix = QInputDialog::getText(this, "Prefix AtomTypes", "Enter prefix to apply to all selected AtomTypes", QLineEdit::Normal, "", &ok);
	if (!ok) return;

	QList<QListWidgetItem*> selectedItems = ui_.AtomTypesList->selectedItems();
	QList<QListWidgetItem*>::iterator i;
	for (i = selectedItems.begin(); i != selectedItems.end(); ++i)
	{
		AtomType* at = VariantPointer<AtomType>((*i)->data(Qt::UserRole));
		at->setName(CharString("%s%s", qPrintable(prefix), at->name()));
	}

	updateAtomTypesPage();
}

void ImportSpeciesWizard::on_AtomTypesSuffixButton_clicked(bool checked)
{
	bool ok;
	QString suffix = QInputDialog::getText(this, "Suffix AtomTypes", "Enter suffix to apply to all selected AtomTypes", QLineEdit::Normal, "", &ok);
	if (!ok) return;

	QList<QListWidgetItem*> selectedItems = ui_.AtomTypesList->selectedItems();
	QList<QListWidgetItem*>::iterator i;
	for (i = selectedItems.begin(); i != selectedItems.end(); ++i)
	{
		AtomType* at = VariantPointer<AtomType>((*i)->data(Qt::UserRole));
		at->setName(CharString("%s%s", at->name(), qPrintable(suffix)));
	}

	updateAtomTypesPage();
}

/*
 * MasterTerms Page
 */

// Row update function for MasterTermsList
void ImportSpeciesWizard::updateMasterTermsTreeChild(QTreeWidgetItem* parent, int childIndex, MasterIntra* masterIntra, bool createItem)
{
	QTreeWidgetItem* item;
	if (createItem)
	{
		item = new QTreeWidgetItem;
		item->setData(0, Qt::UserRole, VariantPointer<MasterIntra>(masterIntra));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		parent->insertChild(childIndex, item);
	}
	else item = parent->child(childIndex);

	// Set item data
	item->setText(0, masterIntra->name());
	item->setIcon(0, QIcon(dissolveReference_->constCoreData().findMasterTerm(masterIntra->name()) ?  ":/general/icons/general_warn.svg" : ":/general/icons/general_true.svg"));
}

// Update page with MasterTerms in our temporary Dissolve reference
void ImportSpeciesWizard::updateMasterTermsPage()
{
	// Update the list against the global MasterTerm tree
	TreeWidgetUpdater<ImportSpeciesWizard,MasterIntra> bondUpdater(masterBondItemParent_, temporaryCoreData_.masterBonds(), this, &ImportSpeciesWizard::updateMasterTermsTreeChild);
	TreeWidgetUpdater<ImportSpeciesWizard,MasterIntra> angleUpdater(masterAngleItemParent_, temporaryCoreData_.masterAngles(), this, &ImportSpeciesWizard::updateMasterTermsTreeChild);
	TreeWidgetUpdater<ImportSpeciesWizard,MasterIntra> torsionUpdater(masterTorsionItemParent_, temporaryCoreData_.masterTorsions(), this, &ImportSpeciesWizard::updateMasterTermsTreeChild);

	// Determine whether we have any naming conflicts
	bool conflicts = false;
	ListIterator<MasterIntra> bondIterator(temporaryCoreData_.masterBonds());
	while (MasterIntra* intra = bondIterator.iterate()) if (dissolveReference_->constCoreData().findMasterTerm(intra->name()))
	{
		conflicts = true;
		break;
	}
	ListIterator<MasterIntra> angleIterator(temporaryCoreData_.masterAngles());
	while (MasterIntra* intra = angleIterator.iterate()) if (dissolveReference_->constCoreData().findMasterTerm(intra->name()))
	{
		conflicts = true;
		break;
	}
	ListIterator<MasterIntra> torsionIterator(temporaryCoreData_.masterTorsions());
	while (MasterIntra* intra = torsionIterator.iterate()) if (dissolveReference_->constCoreData().findMasterTerm(intra->name()))
	{
		conflicts = true;
		break;
	}
	ui_.MasterTermsIndicator->setNotOK(conflicts);
	if (conflicts) ui_.MasterTermsIndicatorLabel->setText("One or more MasterTerms in the imported Species conflict with existing ones");
	else ui_.MasterTermsIndicatorLabel->setText("There are no naming conflicts with the imported MasterTerms");
}

void ImportSpeciesWizard::on_MasterTermsTree_itemSelectionChanged()
{
	// Enable / disable prefix and suffix buttons as appropriate
	bool isSelection = ui_.MasterTermsTree->selectedItems().count() > 0;
	ui_.MasterTermsPrefixButton->setEnabled(isSelection);
	ui_.MasterTermsSuffixButton->setEnabled(isSelection);
}

void ImportSpeciesWizard::masterTermsTreeEdited(QWidget* lineEdit)
{
	// Since the signal that leads us here does not tell us the item that was edited, update all MasterTerm names here before updating the page
	for (int n=0; n<masterBondItemParent_->childCount(); ++n)
	{
		QTreeWidgetItem* item = masterBondItemParent_->child(n);
		MasterIntra* intra = VariantPointer<MasterIntra>(item->data(0, Qt::UserRole));
		if (!intra) continue;

		intra->setName(qPrintable(item->text(0)));
	}
	for (int n=0; n<masterAngleItemParent_->childCount(); ++n)
	{
		QTreeWidgetItem* item = masterAngleItemParent_->child(n);
		MasterIntra* intra = VariantPointer<MasterIntra>(item->data(0, Qt::UserRole));
		if (!intra) continue;

		intra->setName(qPrintable(item->text(0)));
	}
	for (int n=0; n<masterTorsionItemParent_->childCount(); ++n)
	{
		QTreeWidgetItem* item = masterTorsionItemParent_->child(n);
		MasterIntra* intra = VariantPointer<MasterIntra>(item->data(0, Qt::UserRole));
		if (!intra) continue;

		intra->setName(qPrintable(item->text(0)));
	}

	updateMasterTermsPage();
}

void ImportSpeciesWizard::on_MasterTermsPrefixButton_clicked(bool checked)
{
	bool ok;
	QString prefix = QInputDialog::getText(this, "Prefix MasterTerms", "Enter prefix to apply to all selected MasterTerms", QLineEdit::Normal, "", &ok);
	if (!ok) return;

	QList<QTreeWidgetItem*> selectedItems = ui_.MasterTermsTree->selectedItems();
	QList<QTreeWidgetItem*>::iterator i;
	for (i = selectedItems.begin(); i != selectedItems.end(); ++i)
	{
		MasterIntra* intra = VariantPointer<MasterIntra>((*i)->data(0, Qt::UserRole));
		intra->setName(CharString("%s%s", qPrintable(prefix), intra->name()));
	}

	updateMasterTermsPage();
}

void ImportSpeciesWizard::on_MasterTermsSuffixButton_clicked(bool checked)
{
	bool ok;
	QString suffix = QInputDialog::getText(this, "Suffix MasterTerms", "Enter suffix to apply to all selected MasterTerms", QLineEdit::Normal, "", &ok);
	if (!ok) return;

	QList<QTreeWidgetItem*> selectedItems = ui_.MasterTermsTree->selectedItems();
	QList<QTreeWidgetItem*>::iterator i;
	for (i = selectedItems.begin(); i != selectedItems.end(); ++i)
	{
		MasterIntra* intra = VariantPointer<MasterIntra>((*i)->data(0, Qt::UserRole));
		intra->setName(CharString("%s%s", intra->name(), qPrintable(suffix)));
	}

	updateMasterTermsPage();
}

/*
 * Species Name Page
 */

// Species name edited
void ImportSpeciesWizard::on_SpeciesNameEdit_textChanged(const QString text)
{
	if (!dissolveReference_)
	{
		printf("Internal Error: Dissolve pointer has not been set in ImportSpeciesWizard.\n");
		return;
	}

	// Name of the prospective Species has been changed, so make sure it is valid
	bool readyForImport;
	if (text.isEmpty()) readyForImport = false;
	else readyForImport = dissolveReference_->findSpecies(qPrintable(text)) == NULL;

	ui_.SpeciesNameIndicator->setOK(readyForImport);

	// Update state of progression controls
	updateProgressionControls();
}
