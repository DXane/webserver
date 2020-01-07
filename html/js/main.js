//Fügt in den Footer Tag die globalen Werte ein
function insertfooter(){
    var foot=document.getElementsByTagName("footer")[0];
    foot.innerHTML+="Daniel Förderer &copy;2019 | Seite erstellt mit <a href=\"https://getbootstrap.com\">Bootstrap 4</a> | <a href=\"http://www.github.com\">Github</a> | Hintergrund erstellt mit <a href=\"http://weavesilk.com/\">Silk</a>";
    foot.className="col-md-12 col-sm-12 text-center";
    foot.style.marginbottom="-50px";
}

/*function fixedbottom(){
    var foot=document.getElementsByTagName("footer")[0];
    foot.classList.add('fixed-bottom');
}*/
//Fügt die globale Navbar ein
function insertnav(){
    var list=document.getElementsByTagName("nav");
    var navbar=list[0];
    navbar.innerHTML="<div class=\"col-md-1 col-sm-1\"><img src=\"./img/albefologo.jpeg\" alt=\"Logo\" id=\"logo\" class=\"rounded-circle img-fluid logo navlogo\"></div>";
    navbar.innerHTML+="<a class=\"col-md-2 col-sm-2 navitem\" href=\"index.html\">Home</a>";
    navbar.innerHTML+="<a class=\"col-md-2 col-sm-2 navitem\" href=\"command.html\"> Command</a>";
    navbar.innerHTML+="<a class=\"col-md-2 col-sm-2 navitem\" href=\"leaderbd.html\">Leaderboard</a>";
    navbar.innerHTML+="<a class=\"col-md-2 col-sm-2 navitem\" href=\"getbot.html\">Get It</a>";
    navbar.innerHTML+="<a class=\"col-md-2 col-sm-2 navitem\" href=\"kontakt.html\">Kontakt</a>";
    navbar.innerHTML+="<div class=\"col-md-1 col-sm-1\"><a href=\"https://www.github.com\"><img src=\"./img/githublogo.png\" alt=\"Github\" id=\"git\" class=\"img-fluid navlogo\"></a></div>";
}
//Fügt alle globalen Elemente ein
function insertessential(){
    insertfooter();
    insertnav();
}
$(document).ready(function(){
    
    $('article#discinv').hover(function(){
        $('#discordinvite').removeClass('d-none');
        $('#hubinvite').addClass('d-none');
    });
    $('article#gitinv').hover(function(){
        $('#discordinvite').addClass('d-none');
        $('#hubinvite').removeClass('d-none');
    });
    $('#submit').click(function(){
        $('#messagebox').removeClass('d-none');
        //$('#msgbtn').removeClass('d-none');
    });
    $('#msgbtn').click(function(){
        $('#messagebox').addClass('d-none');
        //$('#msgbtn').addClass('d-none');
    });
});


function kontaktformular(){
    var form = document.forms[0];
    var name=form.elements["fvorname"].value;
    var email=form.elements["femail"].value;
    var textbox=form.elements["ftext"].value;
    var newsletter=form.elements["fnewsletter"].checked;
    var box= document.getElementById("message");
    box.innerText="Danke "+name+" für ihre Nachricht! Wir werden uns bei ihnen demnächst über die Email:"+email+" melden.";
    if(!newsletter){
        box.innerText+="Um immer aktuell zu sein abonnieren sie noch unseren Newsletter.";
    }
    else{
        box.innerText+="\nDanke das sie unseren Newsletter abonniert haben! Um ihn zu verlassen schreiben sie uns einfach eine Mail!";
    }
    console.log(textbox, newsletter);
    return false;
}

function hide(className){
    var obj=document.getElementsByClassName(className);
    for(var i=0;i<obj.length;i++){
        console.log(obj[i]);
        obj[i].classList.add('d-none');
    }
}

function show(className){
    var obj=document.getElementsByClassName(className);
    for(var i=0;i<obj.length;i++){
        console.log(obj[i]);
        obj[i].classList.remove('d-none');
    }
}

function toggle(className){
    var filter=document.getElementById("filter"+className);
    console.log(filter.checked==true);
    console.log(filter)
    if(filter.checked == true ? show(className) : hide(className));
}