function doGet(e) { 
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet(); 
  var id = e.parameter.id; 
  var name = e.parameter.name; 
  var access = e.parameter.access; 
 
  // Log data to sheet 
  sheet.appendRow([new Date(), id, name, access]); 
 
  // Email setup 
  var email = "sinchanashetty27495@gmail.com"; //     Your email 
  var subject = ""; 
  var body = ""; 
 
  if (name === "Unauthorized") { 
    subject = "   Unauthorized Access Attempt"; 
    body = "An unauthorized ID [" + id + "] tried to access."; 
  } else if (access === "Denied") { 
    subject = "  Access Denied"; 
    body = name + " failed to log in after 6 password attempts."; 
  } else if (access === "Granted") { 
    subject = "   Access Granted"; 
body = name + " has successfully accessed the system."; 
} 
// Send email only if subject is set 
if (subject !== "") { 
MailApp.sendEmail(email, subject, body); 
} 
return ContentService.createTextOutput("Success"); 
}
