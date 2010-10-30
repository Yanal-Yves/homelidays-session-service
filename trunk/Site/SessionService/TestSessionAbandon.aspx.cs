using System;
using Homelidays.Web.SessionService;

public partial class SessionService_TestSessionAbandon : SessionPage
{
    protected void Page_Load(object sender, EventArgs e)
    {
        this.AbandonSession();
    }
}
