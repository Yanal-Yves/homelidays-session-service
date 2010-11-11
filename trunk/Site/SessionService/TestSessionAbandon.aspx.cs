using System;
using Homelidays.Web.SessionService;

/// <summary>
/// This page test the AbandonSession method.
/// </summary>
public partial class SessionService_TestSessionAbandon : SessionPage
{
    /// <summary>
    /// Standard Page_Load ASPX event.
    /// </summary>
    /// <param name="sender">The current page</param>
    /// <param name="e">Event Argument.</param>
    protected void Page_Load(object sender, EventArgs e)
    {
        this.AbandonSession();
    }
}
