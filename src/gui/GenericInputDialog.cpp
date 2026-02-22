#include "GenericInputDialog.h"
#include "fkt_GPIB.h"


GenericInputDialog::GenericInputDialog(
    wxWindow*                                         parent,
    const wxString&                                   title,
    const std::vector<InputFieldDef>&                 fieldDefs,
    std::function<void(const std::vector<wxString>&)> onConfirm)
    : wxDialog(parent, wxID_ANY, title,
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_onConfirm(onConfirm)
{
    m_fieldDefs = fieldDefs;

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Grid: 2 columns — label | value input
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(2, 8, 3);
    gridSizer->AddGrowableCol(1, 1);

    for (const auto& field : fieldDefs)
    {
        wxStaticText* label = new wxStaticText(this, wxID_ANY, field.label);

        wxTextCtrl* input = new wxTextCtrl(this, wxID_ANY, field.defaultValue,
                                           wxDefaultPosition, wxSize(180, -1));

        gridSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        gridSizer->Add(input, 1, wxEXPAND | wxALL, 5);
        m_inputs.push_back(input);
    }

    mainSizer->Add(gridSizer, 1, wxEXPAND | wxALL, 10);

    // Horizontal separator
    mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Confirm / Cancel button row
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* cancelBtn  = new wxButton(this, wxID_CANCEL, "Cancel");
    wxButton* confirmBtn = new wxButton(this, wxID_ANY,    "Confirm");
    confirmBtn->SetDefault();
    confirmBtn->Bind(wxEVT_BUTTON, &GenericInputDialog::OnConfirm, this);
    btnSizer->AddStretchSpacer(1);
    btnSizer->Add(cancelBtn,  0, wxALL, 5);
    btnSizer->Add(confirmBtn, 0, wxALL, 5);
    mainSizer->Add(btnSizer, 0, wxEXPAND | wxALL, 5);

    SetSizer(mainSizer);
    Fit();
    Centre(wxBOTH);
}

std::vector<wxString> GenericInputDialog::GetValues() const
{
    std::vector<wxString> values;
    values.reserve(m_inputs.size());
    for (const auto* ctrl : m_inputs)
        values.push_back(ctrl->GetValue());
    return values;
}

void GenericInputDialog::OnConfirm(wxCommandEvent& /*event*/)
{
    std::string cmdText;
    std::vector<wxString> values = GetValues();

    // write set values to device
    for (size_t i = 0; i < values.size(); i++)
    {
        cmdText += m_fieldDefs[i].gpibCommand;
        cmdText += " ";
        cmdText += values[i];
        cmdText += ";";
        if (i < values.size() - 1)
        {
            cmdText += " :"; // Add to chain commands together 
        }
    }
    
    if (PrologixUsbGpibAdapter::get_instance().getConnected())
    {
        PrologixUsbGpibAdapter::get_instance().write(cmdText);
    }
    else
    {
        std::cerr << "Not Connected to Adapter couldn't send Set window input" << std::endl;
    }

    if (m_onConfirm)
    {
        m_onConfirm(GetValues());
    }
    EndModal(wxID_OK);

    // TODO add check if Plotter is connected
}
