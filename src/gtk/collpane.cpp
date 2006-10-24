/////////////////////////////////////////////////////////////////////////////
// Name:        src/gtk/collpane.cpp
// Purpose:     wxCollapsiblePane
// Author:      Francesco Montorsi
// Modified By:
// Created:     8/10/2006
// Id:          $Id$
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __WXGTK24__

#include "wx/collpane.h"
#include "wx/gtk/private.h"

#include <gtk/gtkexpander.h>

const wxChar wxCollapsiblePaneNameStr[] = wxT("CollapsiblePane");

// ============================================================================
// implementation
// ============================================================================

//-----------------------------------------------------------------------------
// "notify::expanded" signal
//-----------------------------------------------------------------------------

extern "C" {

static void gtk_collapsiblepane_expanded_callback (GObject    *object,
                                                    GParamSpec *param_spec,
                                                    wxCollapsiblePane *p)
{
    // NB: unlike for the "activate" signal, when this callback is called, if
    //     we try to query the "collapsed" status through p->IsCollapsed(), we
    //     get the right value. I.e. here p->IsCollapsed() will return false if
    //     this callback has been called at the end of a collapsed->expanded
    //     transition and viceversa. Inside the "activate" signal callback
    //     p->IsCollapsed() would return the wrong value!

    wxSize sz;
    if ( p->IsExpanded() )
    {
        // unfortunately there's no clean way to retrieve the minimal size of
        // the expanded pane in this handler or in other handlers for the
        // signals generated by user clicks on the GtkExpander button:
        // p->GetBestSize() or p->GetMinSize() would still return the size for
        // the collapsed expander even if the collapsed->expanded transition
        // has already been completed (this because GTK+ queues some resize
        // calls which still must be processed). So, the only solution to
        // correctly set the size hints for this window is to calculate the
        // expanded size ourselves, without relying on p->Get[Best|Min]Size:
        sz = p->GetMinSize();
        sz.SetWidth(wxMax(sz.x, p->GetPane()->GetMinSize().x));
        sz.SetHeight(sz.y + p->GetPane()->GetMinSize().y + 10);
    }
    else // collapsed
    {
        // same problem described above: using p->Get[Best|Min]Size() here we
        // would get the size of the control when it is expanded even if the
        // expanded->collapsed transition should be complete now...
        // So, we use the size cached at control-creation time...
        sz = p->m_szCollapsed;
    }

    p->OnStateChange(sz);

    if ( p->m_bIgnoreNextChange )
    {
        // change generated programmatically - do not send an event!
        p->m_bIgnoreNextChange = false;
        return;
    }

    // fire an event
    wxCollapsiblePaneEvent ev(p, p->GetId(), p->IsCollapsed());
    p->GetEventHandler()->ProcessEvent(ev);
}
}

static void
gtk_collapsiblepane_insert_callback(wxWindowGTK* parent, wxWindowGTK* child)
{
    // this callback should be used only once to insert the "pane" into the
    // GtkExpander widget. wxGenericCollapsiblePane::DoAddChild() will check if
    // it has been called only once (and in any case we would get a warning
    // from the following call as GtkExpander is a GtkBin and can contain only
    // a single child!).
    gtk_container_add (GTK_CONTAINER (parent->m_widget), child->m_widget);
}

//-----------------------------------------------------------------------------
// wxCollapsiblePane
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxCollapsiblePane, wxGenericCollapsiblePane)

BEGIN_EVENT_TABLE(wxCollapsiblePane, wxGenericCollapsiblePane)
    EVT_SIZE(wxCollapsiblePane::OnSize)
END_EVENT_TABLE()

bool wxCollapsiblePane::Create(wxWindow *parent,
                               wxWindowID id,
                               const wxString& label,
                               const wxPoint& pos,
                               const wxSize& size,
                               long style,
                               const wxValidator& val,
                               const wxString& name)
{
    if (gtk_check_version(2,4,0))
        return wxGenericCollapsiblePane::Create(parent, id, label,
                                                pos, size, style, val, name);

    m_needParent = true;
    m_acceptsFocus = true;
    m_bIgnoreNextChange = false;

    if ( !PreCreation( parent, pos, size ) ||
          !wxControl::CreateBase(parent, id, pos, size, style, val, name) )
    {
        wxFAIL_MSG( wxT("wxCollapsiblePane creation failed") );
        return false;
    }

    m_widget =
        gtk_expander_new_with_mnemonic(wxGTK_CONV(GTKConvertMnemonics(label)));

    // see the gtk_collapsiblepane_expanded_callback comments to understand why
    // we connect to the "notify::expanded" signal instead of the more common
    // "activate" one
    g_signal_connect(m_widget, "notify::expanded",
                     G_CALLBACK(gtk_collapsiblepane_expanded_callback), this);

    // before creating m_pPane, we need to makesure our own insert callback
    // will be used
    m_insertCallback = gtk_collapsiblepane_insert_callback;

    // this the real "pane"
    m_pPane = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                           wxNO_BORDER);

    gtk_widget_show( GTK_WIDGET(m_widget) );
    m_parent->DoAddChild( this );

    PostCreation(size);

    // remember the size of this control when it's collapsed
    m_szCollapsed = GetBestSize();

    return true;
}

wxSize wxCollapsiblePane::DoGetBestSize() const
{
    if (!gtk_check_version(2,4,0))
    {
        wxASSERT_MSG( m_widget, wxT("DoGetBestSize called before creation") );

        GtkRequisition req;
        req.width = 2;
        req.height = 2;
        (* GTK_WIDGET_CLASS( GTK_OBJECT_GET_CLASS(m_widget) )->size_request )
                (m_widget, &req );

        // notice that we do not cache our best size here as it changes
        return wxSize(req.width, req.height);
    }

    return wxGenericCollapsiblePane::DoGetBestSize();
}

void wxCollapsiblePane::Collapse(bool collapse)
{
    if (!gtk_check_version(2,4,0))
    {
        // optimization
        if (IsCollapsed() == collapse)
            return;

        // do not send event in next signal handler call
        m_bIgnoreNextChange = true;
        gtk_expander_set_expanded(GTK_EXPANDER(m_widget), !collapse);
    }
    else
        wxGenericCollapsiblePane::Collapse(collapse);
}

bool wxCollapsiblePane::IsCollapsed() const
{
    if (!gtk_check_version(2,4,0))
        return !gtk_expander_get_expanded(GTK_EXPANDER(m_widget));

    return wxGenericCollapsiblePane::IsCollapsed();
}

void wxCollapsiblePane::SetLabel(const wxString &str)
{
    if (!gtk_check_version(2,4,0))
        gtk_expander_set_label(GTK_EXPANDER(m_widget), wxGTK_CONV(str));
    else
        wxGenericCollapsiblePane::SetLabel(str);
}

void wxCollapsiblePane::OnSize(wxSizeEvent &ev)
{
#if 0       // for debug only
    wxClientDC dc(this);
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(wxPoint(0,0), GetSize());
    dc.SetPen(*wxRED_PEN);
    dc.DrawRectangle(wxPoint(0,0), GetBestSize());
#endif

    // here we need to resize the pane window otherwise, even if the GtkExpander container
    // is expanded or shrinked, the pane window won't be updated!
    m_pPane->SetSize(ev.GetSize());

    // we need to explicitely call m_pPane->Layout() or else it won't correctly relayout
    // (even if SetAutoLayout(true) has been called on it!)
    m_pPane->Layout();
}

#endif //  __WXGTK24__
