#include "CoordinatePanel.h"
#include <cmath>
#include <wx/dcbuffer.h>

CoordinatePanel::CoordinatePanel(wxWindow* parent) : wxPanel(parent) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    // Set the requested window size
    SetMinSize(wxSize(400, 380));
    SetSize(wxSize(400, 380));

    Bind(wxEVT_PAINT, &CoordinatePanel::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &CoordinatePanel::OnMouseClick, this);
    
    // Set coordinate system to match requested range
    m_minX = 0.0; m_maxX = 400.0;
    m_minY = 0.0; m_maxY = 380.0;
}

wxPoint CoordinatePanel::CoordToScreen(double x, double y) {
    wxSize size = GetClientSize();
    
    // Margins to allow space for axis labels
    int marginLeft = 45;
    int marginBottom = 35;
    int marginRight = 20;
    int marginTop = 20;
    
    int drawableWidth = size.x - marginLeft - marginRight;
    int drawableHeight = size.y - marginBottom - marginTop;

    // Linear transformation: (value / range) * pixel_width
    int screenX = marginLeft + ((x - m_minX) / (m_maxX - m_minX)) * drawableWidth;
    int screenY = (size.y - marginBottom) - ((y - m_minY) / (m_maxY - m_minY)) * drawableHeight;
    
    return wxPoint(screenX, screenY);
}

void CoordinatePanel::ScreenToCoord(int px, int py, double& outX, double& outY) {
    wxSize size = GetClientSize();
    
    // Must match the margins in CoordToScreen
    int marginLeft = 45;
    int marginBottom = 35;
    int marginRight = 20;
    int marginTop = 20;
    
    int drawableWidth = size.x - marginLeft - marginRight;
    int drawableHeight = size.y - marginBottom - marginTop;

    // Prevent division by zero if window is tiny
    if (drawableWidth <= 0 || drawableHeight <= 0) return;

    // 1. Convert Screen X back to World X
    // Formula: x = minX + ((screenX - margin) / pixelWidth) * range
    double xPercent = (double)(px - marginLeft) / drawableWidth;
    outX = m_minX + (xPercent * (m_maxX - m_minX));

    // 2. Convert Screen Y back to World Y (remember Y is inverted on screen)
    // Formula: y = minY + ((screenHeight - margin - screenY) / pixelHeight) * range
    double yPercent = (double)((size.y - marginBottom) - py) / drawableHeight;
    outY = m_minY + (yPercent * (m_maxY - m_minY));

    // 3. Optional: Clamp to bounds so clicking outside the box doesn't give -50
    if (outX < m_minX) outX = m_minX;
    if (outX > m_maxX) outX = m_maxX;
    if (outY < m_minY) outY = m_minY;
    if (outY > m_maxY) outY = m_maxY;
}

void CoordinatePanel::OnMouseClick(wxMouseEvent& event) {
    // Only act if a callback is registered
    if (m_onClick) {
        double x, y;
        ScreenToCoord(event.GetX(), event.GetY(), x, y);
        
        // Trigger the callback with the calculated coordinates
        m_onClick(x, y);
    }
    event.Skip();
}

void CoordinatePanel::AddPoint(double x, double y, wxColour color) {
    m_points.push_back({x, y, color});
    Refresh();
}

void CoordinatePanel::ClearPoints() {
    m_points.clear();
    Refresh();
}

void CoordinatePanel::OnPaint(wxPaintEvent& evt) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    
    // Draw Grid Lines every 50 units
    dc.SetPen(wxPen(wxColour(230, 230, 230), 1));
    int step = 50;

    for (int i = (int)m_minX; i <= (int)m_maxX; i += step) {
        dc.DrawLine(CoordToScreen(i, m_minY), CoordToScreen(i, m_maxY));
    }
    for (int i = (int)m_minY; i <= (int)m_maxY; i += step) {
        dc.DrawLine(CoordToScreen(m_minX, i), CoordToScreen(m_maxX, i));
    }
    
    // Draw Main Axes
    dc.SetPen(wxPen(*wxBLACK, 2));
    wxPoint origin = CoordToScreen(0, 0);
    wxPoint xEnd = CoordToScreen(m_maxX, 0);
    wxPoint yEnd = CoordToScreen(0, m_maxY);
    
    dc.DrawLine(origin, xEnd);
    dc.DrawLine(origin, yEnd);
    
    // Axis Labels
    dc.SetTextForeground(*wxBLACK);
    wxFont font = dc.GetFont();
    font.SetPointSize(7);
    dc.SetFont(font);
    
    // X-axis numbers
    for (int i = (int)m_minX; i <= (int)m_maxX; i += step) {
        wxPoint p = CoordToScreen(i, 0);
        dc.DrawText(wxString::Format("%d", i), p.x - 10, p.y + 5);
    }
    
    // Y-axis numbers
    for (int i = (int)m_minY; i <= (int)m_maxY; i += step) {
        if (i == 0) continue; 
        wxPoint p = CoordToScreen(0, i);
        dc.DrawText(wxString::Format("%d", i), p.x - 35, p.y - 7);
    }
    
    // Draw Points
    for (const auto& pt : m_points) {
        wxPoint screenPt = CoordToScreen(pt.x, pt.y);
        
        // Only draw if within visible range
        if (screenPt.x >= 0 && screenPt.y >= 0) {
            dc.SetBrush(wxBrush(pt.color));
            dc.SetPen(wxPen(pt.color, 1));
            dc.DrawCircle(screenPt, 3);
            
            // Optional: coordinate label
            dc.SetTextForeground(pt.color);
            dc.DrawText(wxString::Format("(%.0f,%.0f)", pt.x, pt.y), 
                       screenPt.x + 5, screenPt.y - 12);
        }
    }
}