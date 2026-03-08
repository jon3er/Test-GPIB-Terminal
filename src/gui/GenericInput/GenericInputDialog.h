#pragma once
#include <wx/wx.h>
#include <wx/statline.h>
#include <vector>
#include <functional>
#include <string>

/**
 * @brief Definition of a single row in GenericInputDialog.
 *
 * - label        : human-readable parameter name shown in the left column.
 * - defaultValue : pre-filled value shown in the editable centre column.
 * - gpibCommand  : SCPI/GPIB command template shown read-only in the right
 *                  column.  Use "{}" as a placeholder for the entered value,
 *                  e.g. "SENS:FREQ:STAR {}" becomes "SENS:FREQ:STAR 1e6".
 *                  Leave empty if no command is associated.
 */
struct InputFieldDef
{
    wxString label;
    wxString defaultValue;
    wxString gpibCommand;   ///< SCPI command template ("{}" = value placeholder)
};

/**
 * @brief A reusable modal dialog that renders a configurable list of
 *        label + text-input rows and calls a user-supplied callback on confirm.
 *
 * Usage:
 * @code
 *   std::vector<InputFieldDef> fields = {
 *       { "Start Frequency:", "50"  },
 *       { "Stop Frequency:",  "100" },
 *       // ...up to as many as needed
 *   };
 *
 *   auto* dlg = new GenericInputDialog(this, "My Dialog", fields,
 *       [this](const std::vector<wxString>& vals)
 *       {
 *           // vals[0] = first field value, vals[1] = second, ...
 *       });
 *
 *   dlg->ShowModal();
 *   dlg->Destroy();
 * @endcode
 */
class GenericInputDialog : public wxDialog
{
public:
    /**
     * @param parent     Parent window.
     * @param title      Dialog title bar text.
     * @param fieldDefs  Ordered list of {label, defaultValue} pairs.
     *                   Determines the number of rows displayed.
     * @param onConfirm  Callback invoked when the user clicks Confirm.
     *                   Receives a vector of the current input values in the
     *                   same order as fieldDefs.
     */
    GenericInputDialog(wxWindow*                                         parent,
                       const wxString&                                   title,
                       const std::vector<InputFieldDef>&                 fieldDefs,
                       std::function<void(const std::vector<wxString>&)> onConfirm);

    /**
     * @brief Returns the current text values of all input fields in order.
     *        Can be called after ShowModal() as an alternative to the callback.
     */
    std::vector<wxString> GetValues() const;

    /**
     * @brief Returns the original field definitions (including GPIB commands).
     *        Useful for building the final SCPI strings after ShowModal().
     */
    const std::vector<InputFieldDef>& GetFieldDefs() const { return m_fieldDefs; }

private:
    std::vector<InputFieldDef> m_fieldDefs;
    std::vector<wxTextCtrl*>                          m_inputs;
    std::function<void(const std::vector<wxString>&)> m_onConfirm; // TODO entfernen daten müssen nicht weitergeleitet werden
    bool                                              m_isPlotterMesurement;

    void OnConfirm(wxCommandEvent& event);
};
