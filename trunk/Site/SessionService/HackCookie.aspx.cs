using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

public partial class SessionService_HackCookie : System.Web.UI.Page
{
    protected void Page_Load(object sender, EventArgs e)
    {
        for (int i = 0; i < Request.Cookies.Count; i++)
        {
           if (Request.Cookies[i].Name == "Yacht")
            {//Remplace le guid de sessionid par la chaine de caractére Poulet
                HttpCookie cook = Request.Cookies[i];
               cook.Value=cook.Value.Remove(cook.Value.IndexOf("=")+1);
               cook.Value+= "Poulet";
               Response.Cookies.Set(cook);
            }
        }
       
    }
}
