#include <wx/bmpbuttn.h>
#include <wx/clipbrd.h>
#include <wx/sashwin.h>
#include <wx/laywin.h>
#include <wx/textfile.h>
#include <wx/msgdlg.h>
#include "mxHelpWindow.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxUtils.h"
#include "ConfigManager.h"
#include "string_conversions.h"

#define ERROR_PAGE(page) wxString("<I>ERROR</I>: La pagina \"")<<page<<"\" no se encuentra. <br><br> La ayuda de <I>PSeInt</I> aun esta en contruccion."
#include <wx/settings.h>

mxHelpWindow *helpw = NULL;

BEGIN_EVENT_TABLE(mxHelpWindow,wxFrame)
	EVT_CLOSE(mxHelpWindow::OnClose)
	EVT_BUTTON(mxID_HELPW_SHOWTREE, mxHelpWindow::OnShowTree)
	EVT_BUTTON(mxID_HELPW_ALWAYSONTOP, mxHelpWindow::OnAlwaysOnTop)
	EVT_BUTTON(mxID_HELPW_HOME, mxHelpWindow::OnHome)
	EVT_BUTTON(mxID_HELPW_PREV, mxHelpWindow::OnPrev)
	EVT_BUTTON(mxID_HELPW_NEXT, mxHelpWindow::OnNext)
	EVT_BUTTON(mxID_HELPW_COPY, mxHelpWindow::OnCopy)
	EVT_BUTTON(mxID_HELPW_SEARCH, mxHelpWindow::OnSearch)
	EVT_SASH_DRAGGED(wxID_ANY, mxHelpWindow::OnSashDrag)
	EVT_TREE_SEL_CHANGED(wxID_ANY, mxHelpWindow::OnTree)
	EVT_TREE_ITEM_ACTIVATED(wxID_ANY, mxHelpWindow::OnTree)
	EVT_HTML_LINK_CLICKED(wxID_ANY, mxHelpWindow::OnLink)
	EVT_CHAR_HOOK(mxHelpWindow::OnCharHook)
	EVT_TEXT(wxID_ANY,mxHelpWindow::OnText)
	EVT_SIZE(mxHelpWindow::OnMaximize)
END_EVENT_TABLE();


mxHelpWindow::mxHelpWindow(wxString file)
	: wxFrame (main_window,mxID_HELPW, "Ayuda de PSeInt", 
			   wxDefaultPosition, wxSize(700,400),wxDEFAULT_FRAME_STYLE) 
{

	wxPanel *panel= new wxPanel(this);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	bottomSizer = new wxBoxSizer(wxHORIZONTAL);

	index_sash = new wxSashLayoutWindow(this, wxID_ANY,
		wxDefaultPosition, wxSize(200, 30),
		wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
	index_sash->SetDefaultSize(wxSize(120, 1000));
	index_sash->SetOrientation(wxLAYOUT_VERTICAL);
	index_sash->SetAlignment(wxLAYOUT_LEFT);
	index_sash->SetSashVisible(wxSASH_RIGHT, true);
	
	// create and fill index tree
	tree = new wxTreeCtrl(index_sash, wxID_ANY, wxPoint(0,0), wxSize(10,250), wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT);
	
	wxTextFile fil(DIR_PLUS_FILE(config->help_dir,"index"));
	if (fil.Exists()) {
		fil.Open();
		wxTreeItemId root = tree->AddRoot("Temas de Ayuda",0);
		wxTreeItemId node = root;
		unsigned int tabs=0;
		for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
			unsigned int i=0;
			while (i<str.Len() && str[i]=='\t') 
				i++;
			if (i!=0 && str.Len()>i+3) {
				wxString name = _FixW( str.Mid(i+2).AfterFirst(' ') );
				int icon = str[i]-'0';
				if (i==tabs) {
					node=tree->AppendItem(tree->GetItemParent(node),name,icon);
				} else if (i>tabs) {
					node=tree->AppendItem(node,name,icon);
				} else {
					for (unsigned int j=0;j<tabs-i;j++)
						node=tree->GetItemParent(node);
					node=tree->AppendItem(tree->GetItemParent(node),name,icon);
				}
				items[str.Mid(i+2).BeforeFirst(' ')]=node;
				tabs=i;
			}
		}
		fil.Close();
//		tree->Expand(root);
		wxTreeItemIdValue cokkie;
		node = tree->GetFirstChild(root,cokkie);
		while (node.IsOk()) {
			tree->Expand(node);
			node = tree->GetNextSibling(node);
		}
	}
	
	bottomSizer->Add(index_sash,wxSizerFlags().Proportion(0).Expand());
	
	wxString ipath = DIR_PLUS_FILE_2(config->images_path,"ayuda",config->big_icons?"30":"20");
	m_button_tree = new wxBitmapButton(panel, mxID_HELPW_SHOWTREE, wxBitmap(DIR_PLUS_FILE(ipath,"tree.png"),wxBITMAP_TYPE_PNG));
	m_button_index = new wxBitmapButton(panel, mxID_HELPW_HOME, wxBitmap(DIR_PLUS_FILE(ipath,"indice.png"),wxBITMAP_TYPE_PNG));
	m_button_tree->SetToolTip("Mostrar/Ocultar Indice");
	m_button_index->SetToolTip("Ir al indice");
	m_button_prev = new wxBitmapButton(panel, mxID_HELPW_PREV, wxBitmap(DIR_PLUS_FILE(ipath,"anterior.png"),wxBITMAP_TYPE_PNG));
	m_button_prev->SetToolTip("Ir a la pagina anterior");
	m_button_next = new wxBitmapButton(panel, mxID_HELPW_NEXT, wxBitmap(DIR_PLUS_FILE(ipath,"siguiente.png"),wxBITMAP_TYPE_PNG));
	m_button_next->SetToolTip("Ir a la pagina siguiente");
	m_button_atop = new wxBitmapButton(panel, mxID_HELPW_ALWAYSONTOP, wxBitmap(DIR_PLUS_FILE(ipath,"atop.png"),wxBITMAP_TYPE_PNG));
	m_button_atop->SetToolTip("Hacer que la ventana de ayuda permanezca siempre visible (siempre sobre las demás ventanas)");
	wxBitmapButton *button_copy = new wxBitmapButton(panel, mxID_HELPW_COPY, wxBitmap(DIR_PLUS_FILE(ipath,"copiar.png"),wxBITMAP_TYPE_PNG));
	button_copy->SetToolTip("Copiar seleccion");
	topSizer->Add(m_button_tree,wxSizerFlags().Border(wxALL,2));
	topSizer->Add(m_button_index,wxSizerFlags().Border(wxALL,2));
	topSizer->Add(m_button_prev,wxSizerFlags().Border(wxALL,2));
	topSizer->Add(m_button_next,wxSizerFlags().Border(wxALL,2));
	topSizer->Add(m_button_atop,wxSizerFlags().Border(wxALL,2));
	topSizer->Add(button_copy,wxSizerFlags().Border(wxALL,2));
	search_text = new wxTextCtrl(panel,wxID_ANY);
	topSizer->Add(search_text,wxSizerFlags().Border(wxALL,2).Proportion(1).Expand());

	m_button_search = new wxBitmapButton(panel, mxID_HELPW_SEARCH, wxBitmap(DIR_PLUS_FILE(ipath,"buscar.png"),wxBITMAP_TYPE_PNG));
	m_button_search->SetToolTip("Buscar...");
	topSizer->Add(m_button_search,wxSizerFlags().Border(wxALL,2));
	panel->SetSizer(topSizer);
	html = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(400,300));
	bottomSizer->Add(html,wxSizerFlags().Proportion(1).Expand());
	sizer->Add(panel,wxSizerFlags().Proportion(0).Expand());
	sizer->Add(bottomSizer,wxSizerFlags().Proportion(1).Expand());
	SetSizer(sizer);
	bottomSizer->SetItemMinSize(index_sash,200, 10);
	bottomSizer->Layout();
	m_button_search->SetDefault();
	ShowHelp(file);
}

void mxHelpWindow::ShowIndex() {
	ShowHelp("index.html");
}

void mxHelpWindow::ShowHelp(wxString file) {
	html->LoadPage(DIR_PLUS_FILE(config->help_dir,file));
	HashStringTreeItem::iterator it = items.find(file);
	if (it!=items.end()) tree->SelectItem(it->second);
	RepaintButtons();
	Show();
	Raise();
}

void mxHelpWindow::OnClose(wxCloseEvent &evt) {
	Hide();
}

void mxHelpWindow::OnShowTree(wxCommandEvent &event) {
	if (bottomSizer->GetItem(index_sash)->GetMinSize().GetWidth()<10) {
		bottomSizer->SetItemMinSize(index_sash,200, 10);	
	} else {
		bottomSizer->SetItemMinSize(index_sash,0, 10);
	}
	bottomSizer->Layout(); 
	RepaintButtons();
}

void mxHelpWindow::OnAlwaysOnTop(wxCommandEvent &event) {
	if (GetWindowStyleFlag()&wxSTAY_ON_TOP)
		SetWindowStyleFlag(GetWindowStyleFlag()&(~wxSTAY_ON_TOP));
	else
		SetWindowStyleFlag(GetWindowStyleFlag()|wxSTAY_ON_TOP);
	RepaintButtons();
}

void mxHelpWindow::OnHome(wxCommandEvent &event) {
	html->LoadPage(DIR_PLUS_FILE(config->help_dir,"index.html"));
	RepaintButtons();
}

void mxHelpWindow::OnPrev(wxCommandEvent &event) {
	html->HistoryBack();
	RepaintButtons();
}

void mxHelpWindow::OnNext(wxCommandEvent &event) {
	html->HistoryForward();
	RepaintButtons();
}

void mxHelpWindow::OnCopy(wxCommandEvent &event) {
	wxString text = html->SelectionToText()=="" ? html->ToText() : html->SelectionToText() ;
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxTextDataObject(text));
		wxTheClipboard->Close();
	}
}

void mxHelpWindow::OnSearch(wxCommandEvent &event) {
	wxString keyword = search_text->GetValue().MakeUpper();
	if (keyword=="") {
		wxMessageBox("Debe introducir al menos una palabra clave para buscar","Error");
		return;
	}
	html->SetPage("<HTML><HEAD></HEAD><BODY><I><B>Buscando...</B></I></BODY></HTML>");
	HashStringTreeItem::iterator it = items.begin(), ed = items.end();
	wxString result("<HTML><HEAD></HEAD><BODY><I><B>Resultados:</B></I><UL>");
	int count=0;
	wxArrayString searched;
	while (it!=ed) {
		wxString fname = it->first;
		if (fname.Find('#')!=wxNOT_FOUND)
			fname = fname.BeforeFirst('#');
		bool already=false;
		if (!already && wxFileName::FileExists(DIR_PLUS_FILE(config->help_dir,fname))) {
			for (unsigned int i=0;i<searched.GetCount();i++)
				if (searched[i]==fname) {
					already=true;
					break;
				}
			wxTextFile fil(DIR_PLUS_FILE(config->help_dir,fname));
			fil.Open();
			for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
				if (str.MakeUpper().Contains(keyword)) {
					count++;
					result<<"<LI><A href=\""<<it->first<<"\">"<<tree->GetItemText(it->second)<<"</A></LI>";
					fil.Close();
					break;
				}
			}
			fil.Close();
		}
		searched.Add(fname);
		++it;
	}
	result<<"</UL></BODY></HTML>";
	if (count)		
		html->SetPage(result);
	else
		html->SetPage(wxString("<HTML><HEAD></HEAD><BODY><B>No se encontraron coincidencias para \"")<<search_text->GetValue()<<"\".</B></BODY></HTML>");
	RepaintButtons();
}

void mxHelpWindow::OnSearchAll(wxCommandEvent &event) {
}

void mxHelpWindow::OnSashDrag(wxSashEvent& event) {
    //index_sash->SetDefaultSize(wxSize(event.GetDragRect().width, 1000));
	bottomSizer->SetItemMinSize(index_sash,event.GetDragRect().width<150?150:event.GetDragRect().width, 10);
	//GetClientWindow()->Refresh();
	bottomSizer->Layout();
}

void mxHelpWindow::OnTree(wxTreeEvent &event) {
	HashStringTreeItem::iterator it = items.begin();
	while (it->second != event.GetItem())
		++it;
	if (it->first.Find('#')!=wxNOT_FOUND) {
		if (wxFileName::FileExists(DIR_PLUS_FILE(config->help_dir,it->first.BeforeFirst('#'))))
			html->LoadPage(it->first);
		else
			html->SetPage(ERROR_PAGE(it->first));
	} else {
		if (wxFileName::FileExists(DIR_PLUS_FILE(config->help_dir,it->first)))
			html->LoadPage(it->first);
		else
			html->SetPage(ERROR_PAGE(it->first));
	}
	RepaintButtons();
}

void mxHelpWindow::OnLink (wxHtmlLinkEvent &event) {
	if (event.GetLinkInfo().GetHref().StartsWith("example:")) {
		main_window->OpenProgram(DIR_PLUS_FILE(config->examples_dir,event.GetLinkInfo().GetHref().Mid(8)),true);
		if (IsMaximized()) Maximize(false);
		main_window->Raise();
	} else if (event.GetLinkInfo().GetHref().StartsWith("cmd:")){
		wxString cmd = event.GetLinkInfo().GetHref().AfterFirst(':');
		if (cmd=="creator") main_window->OpenProgram("creator.psz");
		else if (cmd=="log") { main_window->ResetInLogMode(); return; }
	} else if (event.GetLinkInfo().GetHref().StartsWith("http://")) {
		wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
	} else {
		wxString fname = event.GetLinkInfo().GetHref().BeforeFirst('#');
		if (fname.Len()) {
			HashStringTreeItem::iterator it = items.find(fname);
			if (it!=items.end())
				tree->SelectItem(it->second);
			if (!wxFileName::FileExists(DIR_PLUS_FILE(config->help_dir,fname)))
				html->SetPage(ERROR_PAGE(fname));
			else
				event.Skip();
		} else
			event.Skip();
	}
	RepaintButtons();
}

void mxHelpWindow::OnCharHook(wxKeyEvent &evt) {
	if (evt.GetKeyCode()==WXK_ESCAPE)
		Close();
	else
		evt.Skip();
}

static void ButtonSetToggled(wxButton *button, bool value) {
	button->SetBackgroundColour(wxSystemSettings::GetColour(value?wxSYS_COLOUR_3DSHADOW:wxSYS_COLOUR_BTNFACE));
}

static void ButtonSetEnabled(wxButton *button, bool value) {
	button->Enable(value);
}

void mxHelpWindow::RepaintButtons ( ) {
	ButtonSetToggled(m_button_tree, bottomSizer->GetItem(index_sash)->GetMinSize().GetWidth()>10);
	ButtonSetToggled(m_button_atop, GetWindowStyleFlag()&wxSTAY_ON_TOP);
	ButtonSetToggled(m_button_index, html->GetOpenedPageTitle()=="Ayuda de PSeInt - Índice");
	ButtonSetEnabled(m_button_prev, html->HistoryCanBack());
	ButtonSetEnabled(m_button_next, html->HistoryCanForward());
	ButtonSetEnabled(m_button_search, !search_text->GetValue().IsEmpty());
}

void mxHelpWindow::OnText (wxCommandEvent & evt) {
	RepaintButtons();
}

void mxHelpWindow::OnMaximize (wxSizeEvent & evt) {
	evt.Skip();
	if (!IsMaximized() || !(GetWindowStyleFlag()&wxSTAY_ON_TOP)) return;
	wxCommandEvent e2;
	OnAlwaysOnTop(e2);
}

