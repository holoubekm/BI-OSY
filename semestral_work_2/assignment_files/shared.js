function MM_findObj(n, d) { //v4.0
  var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
    d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
  if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
  for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
  if(!x && document.getElementById) x=document.getElementById(n); return x;
}
function MM_goto ( url )
 {
   eval ( "this.location='"+url+"'" );
 }
 
function createOption ( text, value )
 {
   var op = document . createElement ( "option" );
   op . text = text;
   op . value = value;
   return ( op );
 }
function createOptGrp ( text )
 {
   var op = document . createElement ( "optgroup" );
   op . label = text;
   return ( op );
 }

var g_ConfirmText = "";
var g_Secret      = null;

function initLinks ( confirmText, secret )
 {
   g_ConfirmText = confirmText;
   g_Secret      = secret;
 }
function secLink ( url )
 {
   if ( ! g_Secret ) alert ( "Secret missing." );
   this.location = url+'&Sec='+g_Secret;   
 }
function confLink ( url )
 {
   var promptText = g_ConfirmText;
   if ( arguments . length > 1 ) promptText = arguments[1];
   if ( ! confirm ( promptText ) ) return;
   secLink ( url );
 }
function gotoLink ( url )
 {
   this . location=url;
 }

function replaceClass ( x, clsEn, clsDis, enabled )
 {
   if ( ! x ) return;
   var cls = x . className;
   var fld = cls . split ( ' ' );
   var i;

   for ( i = 0; i < fld . length; i ++ )
    if ( fld[i] == clsEn || fld[i] == clsDis )
     {
       fld[i] = enabled ? clsEn : clsDis;
       break;
     }
   if ( i == fld . length )
    fld[fld.length] = enabled ? clsEn : clsDis;

   cls = fld . join ( ' ' );
   if ( x . className != cls ) 
    x . className = cls;
 }

function enableInput ( enabled, id )
 {
   var x = MM_findObj ( id );
   
   x . disabled = ! enabled;
   replaceClass ( x, 'std', 'stdDis', enabled );
   replaceClass ( MM_findObj ( id + '_TX' ), '', 'textDis', enabled );
   replaceClass ( MM_findObj ( id + '_UNIT' ), '', 'textDis', enabled );
 }

function enableInputs ( enabled )
 {
   for ( var i = 1; i < arguments . length; i ++ )
    enableInput ( enabled, arguments[i] );
 }
 