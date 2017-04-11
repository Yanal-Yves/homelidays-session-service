using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

/// <summary>
/// In this test page the session id is hacked. The goal is to test that the session service will stay stable even if its input are not the one expected.
/// </summary>
public partial class SessionService_HackCookie : System.Web.UI.Page
{
    /// <summary>
    /// Page load event.
    /// </summary>
    /// <param name="sender">The loading page.</param>
    /// <param name="e">event arg.</param>
    protected void Page_Load(object sender, EventArgs e)
    {
        for (int i = 0; i < Request.Cookies.Count; i++)
        {
            if (Request.Cookies[i].Name == "Yacht")
            { // Remplace le guid de sessionid par la chaine de caractére Poulet
                HttpCookie cook = Request.Cookies[i];
                cook.Value = cook.Value.Remove(cook.Value.IndexOf("=") + 1);
                cook.Value += "Poulet";
                Response.Cookies.Set(cook);
            }
        }
    }
}
