using System;
using Homelidays.Web.SessionService;

/// <summary>
/// Test page that allow to check that no database access is made if no call to Session are made
/// </summary>
public partial class SessionService_TestNoSessionAccess : SessionPage
{
    /// <summary>
    /// Page load event
    /// </summary>
    /// <param name="sender">the page object that raised the load event</param>
    /// <param name="e">The associated event argument</param>
    protected void Page_Load(object sender, EventArgs e)
    {
    }
}
