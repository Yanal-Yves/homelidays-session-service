using System;
using Homelidays.Web.SessionService;

/// <summary>
/// Test page of the Abandon method of the AspSessionService.
/// </summary>
public partial class SessionService_TestSessionAbandon : SessionPage
{
    /// <summary>
    /// Page load event.
    /// </summary>
    /// <param name="sender">The loading page.</param>
    /// <param name="e">event arg.</param>
    protected void Page_Load(object sender, EventArgs e)
    {
        this.AbandonSession();
    }
}
