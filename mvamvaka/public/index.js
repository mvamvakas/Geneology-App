// Put all onload AJAX calls here, and event listeners
var fileTableString;
$(document).ready(function() {
	document.getElementById("displayDB").disabled = false;
	document.getElementById("deleteButt").disabled = false;
	//updateFileTable();
	//loadFiles();
	$("#login").modal("show");
	$("#storeButt").on("click", function (e){
		let thing = document.getElementById("fileTable").innerHTML;
	    thing2 = thing.replace(/<td>/g,"|");
		thing2 = thing2.replace(/<\/td>/g,"");
		thing2 = thing2.replace(/<a>/g,"");
		thing2 = thing2.replace(/<\/a>/g,"");
		thing2 = thing2.replace(/<tr>/g,"");
		thing2 = thing2.replace(/<\/tr>/g,"<br>");
		thing2 = thing2.replace(/<a href=\"\/uploads\//g,"");
		let thing3 = thing2.split(">");
		
	
		let thing4 = [];
		let i = 0;
		for (i in thing3){
			console.log("yo");
			if (isOdd(i) == true){
				console.log(thing3[i]);
				thing3[i] = thing3[i].replace(/<br/g,"");
				thing4.push(thing3[i]);
			}
		}
		let thing5 = JSON.stringify(thing4);
		//let thing = $('#fileTable').innerHTML;
		i = 0;
		let finalThing = [];
		$.ajax({
			type: 'get',
			url: '/deleteFromTables',   //The server endpoint we are connecting to
			success: function (data) {
					for (i in thing4){
						let ob = new Object();
						let ar = thing4[i].split("|");
						ob.fileName = ar[0];
						ob.source = ar[1];
						ob.version = ar[2];
						ob.encoding = ar[3];
						ob.subName = ar[4];
						ob.subAddress = ar[5];
						ob.numIndiv = ar[6];
						ob.numFams = ar[7];
						let thang = JSON.stringify(ob);
						console.log(thang);
						
						$.ajax({
							type: 'get',
							data: ob,
							url: '/addFileDatabase',   
							success: function (data) {
								let file = ob.fileName;
								let obj2 = new Object();
								obj2.fileName = "uploads/" + file;
								$.ajax({
									type: 'get',
									data: obj2,
									url: '/upDateFileTable',   
									success: function (data) {
										isDBEmpty();
										let fName = obj2.fileName;
										let json = JSON.parse(data);
										let i = 0;
										let dummyString = "";
										if (json != "[]"){
											for (i in json){
												console.log(json[i]);
												let o = new Object();
												o.givenName = json[i].givenName;
												o.surname = json[i].surname;
												o.sex = json[i].SEX;
												o.fSize = json[i].fSize;
												o.fileName = ob.fileName;
												console.log(o.fileName);
												
												$.ajax({
													type: 'get',
													data: o,
													url: '/addIndDatabase',   
													success: function (data) {
														//console.log("individual Added");
														isDBEmpty();
													}
												});
												 
											}
										}
									
									}
								});
							}
						});
					}
					
			},
			error: function(data) {
				//console.log(data);
				let node = document.createElement("P");
				let textNode = document.createTextNode("The function failed");
				node.appendChild(textNode);
				document.getElementById("status").appendChild(node);
				
				//console.log("didnt connect"); 
			}
		}); 
		let node = document.createElement("P");
		let textNode = document.createTextNode("Files have been stored");
		node.appendChild(textNode);
		document.getElementById("status").appendChild(node);
	});
	$("#customQ").on("click", function (e){
		let obj = new Object();
		obj.str = document.getElementById("comment").value;
		console.log(obj.str);
		let string = obj.str.split(" ");
		if (string[0] == "SELECT"){
			$.ajax({
				type: 'get',
				data: obj,
				url: '/executeQ',   
				success: function (data) {
					toTableQuery(data);
				}
			});
		}
		
		
	});
	$("#executeQ").on("click", function (e){
		let obj = new Object();
		let x = document.getElementById("queryChoices").value;
		console.log(x);
		if (x == "ViewBySurname"){
			obj.str = "SELECT * FROM INDIVIDUAL ORDER BY surname;";
		}
		else if (x == "ViewByFile"){
			obj.str = "SELECT * FROM INDIVIDUAL,FILE WHERE (FILE.file_id=INDIVIDUAL.source_file AND FILE.file_Name=\"" + document.getElementById("queryFiles").value + "\")";   //"Shakespeare.ged")
		}
		else if (x == "ViewByMale"){
			obj.str = "SELECT * FROM INDIVIDUAL,FILE WHERE (FILE.file_id=INDIVIDUAL.source_file AND FILE.file_Name=\"" + document.getElementById("queryFiles").value + "\" AND INDIVIDUAL.sex=\"M\") ORDER BY surname";
		}
		else if (x == "ViewByFemale"){
			obj.str = "SELECT * FROM INDIVIDUAL,FILE WHERE (FILE.file_id=INDIVIDUAL.source_file AND FILE.file_Name=\"" + document.getElementById("queryFiles").value + "\" AND INDIVIDUAL.sex=\"F\") ORDER BY given_name";
		}
		else if (x == "ViewByNotJohn"){
			obj.str = "SELECT * FROM INDIVIDUAL,FILE WHERE (FILE.file_id=INDIVIDUAL.source_file AND FILE.file_Name=\"" + document.getElementById("queryFiles").value + "\" AND INDIVIDUAL.given_name<>\"John\" AND INDIVIDUAL.given_name<>\"john\") ORDER BY fam_size";
		}
		
		console.log(obj.str);
		
		$.ajax({
			type: 'get',
			data: obj,
			url: '/executeQ',   
			success: function (data) {
				toTableQuery(data);
			}
		});
		
	});
	$("#help").on("click", function (e){
		$("#helpModal").modal("show");
		let obj = new Object();
		let dummyString = "";
		obj.str = "DESCRIBE FILE;"
		$.ajax({
			type: 'get',
			data: obj,
			url: '/executeQ',   
			success: function (data) {
				let i = 0;
				let j = 0;
				for (i in data){
					dummyString = dummyString.concat("<tr>");
					for (j in data[i]){
						dummyString = dummyString.concat("<td>");
						dummyString = dummyString.concat(data[i][j]);
						dummyString = dummyString.concat("</td>");	
					}
					dummyString = dummyString.concat("</tr>");
					
				}
				$('#modalFileTable').html(dummyString);
			}
		});
		let dummyString2 = "";
		let obj2 = new Object();
		obj2.str = "DESCRIBE INDIVIDUAL;"
		$.ajax({
			type: 'get',
			data: obj2,
			url: '/executeQ',   
			success: function (data) {
				let i = 0;
				let j = 0;
				console.log("HERE HERE HERE");
				console.log(dummyString2);
				for (i in data){
					dummyString2 = dummyString2.concat("<tr>");
					for (j in data[i]){
						dummyString2 = dummyString2.concat("<td>");
						dummyString2 = dummyString2.concat(data[i][j]);
						dummyString2 = dummyString2.concat("</td>");	
					}
					dummyString2 = dummyString2.concat("</tr>");
					
				}
				$('#modalIndTable').html(dummyString2);
			}
		});
		
	});
	$("#deleteButt").on("click", function (e){
		$.ajax({
			type: 'get',
			url: '/deleteFromTables',   //The server endpoint we are connecting to
			success: function (data) {
				$("#login").modal("hide");
				let node = document.createElement("P");
				let textNode = document.createTextNode("The database has been cleared");
				node.appendChild(textNode);
				document.getElementById("status").appendChild(node);
				
			},
			error: function(data) {
				//console.log(data);
				let node = document.createElement("P");
				let textNode = document.createTextNode("The function failed");
				node.appendChild(textNode);
				document.getElementById("status").appendChild(node);
				
				//console.log("didnt connect"); 
			}
		}); 
		document.getElementById("deleteButt").disabled = true;
	});
	$("#displayDB").on("click", function (e){
		$.ajax({
			type: 'get',
			url: '/numFileTable',   //The server endpoint we are connecting to
			success: function (data) {
				let i = 0;
				for (i in data){}
				let node = document.createElement("P");
				let textNode = "Database has " + data[i] + " files and ";
				//node.appendChild(textNode);
				//document.getElementById("status").appendChild(node);
				
				$.ajax({
					type: 'get',
					url: '/numIndTable',   //The server endpoint we are connecting to
					success: function (data) {
						let i = 0;
						for (i in data){}
						//let node2 = document.createElement("P");
						let textNode2 = data[i] + " individuals";
						let textNode3 = document.createTextNode(textNode + textNode2);
							 
						node.appendChild(textNode3);
						document.getElementById("status").appendChild(node);
					},
					error: function(data) { 
					}
				});
				
			},
			error: function(data) { 
			}
		});
		 
	});
	
	$("#loginButt").on("click", function (e){
		let obj = new Object();
		obj.username = document.getElementById("userName").value;
		obj.password = document.getElementById("password").value;
		obj.database = document.getElementById("database").value;
		console.log(obj);
		$.ajax({
			type: 'get',
			data: obj,
			url: '/getDatabase',   //The server endpoint we are connecting to
			success: function (data) {
				console.log(data);
				if (data != "fail"){
					$("#login").modal("hide");
					let node = document.createElement("P");
					let textNode = document.createTextNode("Access to database successfull");
					node.appendChild(textNode);
					document.getElementById("status").appendChild(node);
					loadFiles();
				}
				else{
					alert("Login Failed");
				}
			},
			error: function(data) {
				console.log(data);
				/*let node = document.createElement("P");
				let textNode = document.createTextNode("The function failed");
				node.appendChild(textNode);
				document.getElementById("status").appendChild(node);
				*/
				console.log("didnt connect"); 
			}
		});
	});
	$("#desButt").on("click", function (e){
		let obj = new Object();
		let x = document.getElementById("desChoices").value;
		var res = x.split("|");
		obj.givenName = res[0];
		obj.surname = res[1];
		console.log(obj);
		
		let obj2 = {};
		obj2.json = JSON.stringify(obj);
		obj2.file = "uploads/" + document.getElementById("desSelect").value;
		obj2.max = document.getElementById("maxDes").value;
		console.log(obj2);
		
		$.ajax({
			type: 'get',
			data: obj2,            //Request type      //Data type - we will use JSON for almost everything 
			url: '/getDes',   //The server endpoint we are connecting to
			
			success: function (data) {
				let obj = JSON.parse(data);
				//console.log(obj[0][0]);
				//console.log(data);
				let i = 1;
				let k = 0;
				let dummyString = "";
				for (i in obj){
					k++;
				//for (i = 1; i <= obj.length; i++){
					let j = 0;
					//console.log(obj[i]);
					dummyString = dummyString.concat("<tr>");
					dummyString = dummyString.concat("<td>");
					
					dummyString = dummyString.concat(k);
					dummyString = dummyString.concat("<br>");
					dummyString = dummyString.concat("</td>");
					dummyString = dummyString.concat("<td>");
					for (j in obj[i]){
						
						dummyString = dummyString.concat(obj[i][j].givenName + " " + obj[i][j].surname);
						dummyString = dummyString.concat("<br>");
					}
					dummyString = dummyString.concat("</td>");
					dummyString = dummyString.concat("</tr>");
					
				}
				if (dummyString == ""){
					//let dummyString = "";
					dummyString = dummyString.concat("<tr>");
					dummyString = dummyString.concat("<td>");
					
					dummyString = dummyString.concat("1");
					
					dummyString = dummyString.concat("<td>");
					dummyString = dummyString.concat("No Descendants");
					dummyString = dummyString.concat("</td>");
					
					dummyString = dummyString.concat("</td>");
					dummyString = dummyString.concat("</tr>");
					
				}
				$('#desListTable').html(dummyString);
			},
			error: function() {
				let node = document.createElement("P");
				let textNode = document.createTextNode("The function failed");
				node.appendChild(textNode);
				document.getElementById("status").appendChild(node);
				 
			}
		});
		 
	});
	
	
	$("#ansButt").on("click", function (e){
		let obj = new Object();
		let x = document.getElementById("aChoices").value;
		var res = x.split("|");
		obj.givenName = res[0];
		obj.surname = res[1];
		console.log(obj);
		
		let obj2 = {};
		obj2.json = JSON.stringify(obj);
		obj2.file = "uploads/" + document.getElementById("aSelect").value;
		obj2.max = document.getElementById("maxAns").value;
		console.log(obj2);
		
		$.ajax({
			type: 'get',
			data: obj2,            //Request type      //Data type - we will use JSON for almost everything 
			url: '/getAns',   //The server endpoint we are connecting to
			
			success: function (data) {
				let obj = JSON.parse(data);
				//console.log(obj[0][0]);
				//console.log(data);
				let i = 1;
				let k = 0;
				let dummyString = "";
				for (i in obj){
					k++;
				//for (i = 1; i <= obj.length; i++){
					let j = 0;
					//console.log(obj[i]);
					dummyString = dummyString.concat("<tr>");
					dummyString = dummyString.concat("<td>");
					
					dummyString = dummyString.concat(k);
					dummyString = dummyString.concat("<br>");
					dummyString = dummyString.concat("</td>");
					dummyString = dummyString.concat("<td>");
					for (j in obj[i]){
						
						dummyString = dummyString.concat(obj[i][j].givenName + " " + obj[i][j].surname);
						dummyString = dummyString.concat("<br>");
					}
					dummyString = dummyString.concat("</td>");
					dummyString = dummyString.concat("</tr>");
					
				}
				if (dummyString == ""){
					//let dummyString = "";
					dummyString = dummyString.concat("<tr>");
					dummyString = dummyString.concat("<td>");
					
					dummyString = dummyString.concat("1");
					
					dummyString = dummyString.concat("<td>");
					dummyString = dummyString.concat("No Ancestors");
					dummyString = dummyString.concat("</td>");
					
					dummyString = dummyString.concat("</td>");
					dummyString = dummyString.concat("</tr>");
					
				}
				$('#ansListTable').html(dummyString);
			},
			error: function() {
				let node = document.createElement("P");
				let textNode = document.createTextNode("The function failed");
				node.appendChild(textNode);
				document.getElementById("status").appendChild(node);
			}
		});
		 
	});
	
	
	$("#uploadButt").on("click", function (e){
		let file = $("#upFile")[0].files[0];
		let formData = new FormData();
		formData.append("uploadFile", file);
		$.ajax({
			type: 'post',
			data: formData,            //Request type      //Data type - we will use JSON for almost everything 
			url: '/upload',   //The server endpoint we are connecting to
			processData: false,
			contentType: false,
			
			success: function () {
				let node = document.createElement("P");
				let textNode = document.createTextNode("The file was successfully uploaded");
				node.appendChild(textNode);
				document.getElementById("status").appendChild(node);
				loadFiles();
			},
			error: function() {
				let node = document.createElement("P");
				let textNode = document.createTextNode("The file was not uploaded");
				node.appendChild(textNode);
				document.getElementById("status").appendChild(node);
			}
		});
	});
	
	$("#statusB").on("click", function (e){
		let x = document.getElementById("status");
		x.innerHTML = "";
	});
	
	$("#indButt").on("click", function (e){
		console.log("hi");
		let obj = new Object();
		obj.givenName = document.getElementById("givenNameAdd").value;
		obj.surname = document.getElementById("surNameAdd").value;
		console.log(obj);
		let obj2 = new Object();
		obj2.json = JSON.stringify(obj);
		console.log(obj2.json);
		obj2.fileName = "uploads/" + document.getElementById("addSelect").value;
		$.ajax({
			type: 'get',
			data: obj2,            //Request type      //Data type - we will use JSON for almost everything 
			url: '/addIndiv',   //The server endpoint we are connecting to
			success: function () {
				let node = document.createElement("P");
				let textNode = document.createTextNode("The individual was added to the file");
				node.appendChild(textNode);
				document.getElementById("status").appendChild(node);
				loadFiles();
			},
			error: function() {
				let node = document.createElement("P");
				let textNode = document.createTextNode("The individual was not added to the file");
				node.appendChild(textNode);
				document.getElementById("status").appendChild(node);
			}
		});
	});
    
    $('#createGedcom').on('click', function() {
		let obj = new Object();
		//console.log(document.getElementById("fileNameCreate").value);
		//obj.fileName = document.getElementById("fileNameCreate").value;
		obj.source = "webtreprint.com";
		obj.gedcVersion = "5.5.1";
		obj.encoding = "UTF-8";
		obj.subName = document.getElementById("subNameCreate").value;
		obj.subAddress = document.getElementById("subAddCreate").value;
		var jsonString= JSON.stringify(obj);
		console.log(jsonString);
		let fileName = "uploads/" + document.getElementById("fileNameCreate").value;
		let obj2 = {};
		obj2.fileName = fileName;
		obj2.str = jsonString;
		$.ajax({
			type: 'get',            //Request type
			//dataType: 'json',     //Data type - we will use JSON for almost everything 
			data: obj2,
			url: '/somenewendpoint',   //The server endpoint we are connecting to
			success: function (data) {
				let node = document.createElement("P");
				let textNode = document.createTextNode("The GEDCOM file was created");
				node.appendChild(textNode);
				document.getElementById("status").appendChild(node);
                loadFiles();
			},
			error: function(error) {
				console.log("fail");
				let node = document.createElement("P");
				let textNode = document.createTextNode("The GEDCOM file not was created");
				node.appendChild(textNode);
				document.getElementById("status").appendChild(node);
                loadFiles();
			}
		});
    
	});
	
	

    // Event listener form replacement example, building a Single-Page-App, no redirects if possible
    $('#someform').submit(function(e){
        e.preventDefault();
        $.ajax({});
    });
});

function showCreate(){
	var x = document.getElementById("createDiv");
	var y = document.getElementById("addDiv");
	var z = document.getElementById("uploadDiv");
	var j = document.getElementById("getDDiv");
	var k = document.getElementById("getADiv");
	if (x.style.display == "none"){
		x.style.display = "block";
		y.style.display = "none"
		z.style.display = "none"
		j.style.display = "none"
		k.style.display = "none"
	}
}

function showAdd(){
	var x = document.getElementById("createDiv");
	var y = document.getElementById("addDiv");
	var z = document.getElementById("uploadDiv");
	var j = document.getElementById("getDDiv");
	var k = document.getElementById("getADiv");
	if (y.style.display == "none"){
		x.style.display = "none";
		y.style.display = "block"
		z.style.display = "none"
		j.style.display = "none"
		k.style.display = "none"
	}
}

function showUpload(){
	var x = document.getElementById("createDiv");
	var y = document.getElementById("addDiv");
	var z = document.getElementById("uploadDiv");
	var j = document.getElementById("getDDiv");
	var k = document.getElementById("getADiv");
	if (z.style.display == "none"){
		x.style.display = "none";
		y.style.display = "none"
		z.style.display = "block"
		j.style.display = "none"
		k.style.display = "none"
	}
}

function showD(){
	var x = document.getElementById("createDiv");
	var y = document.getElementById("addDiv");
	var z = document.getElementById("uploadDiv");
	var j = document.getElementById("getDDiv");
	var k = document.getElementById("getADiv");
	if (j.style.display == "none"){
		x.style.display = "none";
		y.style.display = "none"
		z.style.display = "none"
		j.style.display = "block"
		k.style.display = "none"
	}
}

function showA(){
	var x = document.getElementById("createDiv");
	var y = document.getElementById("addDiv");
	var z = document.getElementById("uploadDiv");
	var j = document.getElementById("getDDiv");
	var k = document.getElementById("getADiv");
	if (k.style.display == "none"){
		x.style.display = "none";
		k.style.display = "block"
		z.style.display = "none"
		j.style.display = "none"
		y.style.display = "none"
	}
}

function loadFiles(){
	var x = document.getElementById("fileSelect");
	var y = document.getElementById("addSelect");
	var z = document.getElementById("desSelect");
	var a = document.getElementById("aSelect");
	var b = document.getElementById("fileTable");
	var c = document.getElementById("viewTable");
	let d = document.getElementById("desChoices");
	let e = document.getElementById("aChoices");
	let f = document.getElementById("queryFiles");
	
	x.innerHTML = "";
	y.innerHTML = "";
	z.innerHTML = "";
	a.innerHTML = "";
	b.innerHTML = "";
	c.innerHTML = "";
	d.innerHTML = "";
	e.innerHTML = "";
	f.innerHTML = "";
	//$('#fileTable').innerHTML(dummyString);
	
	$.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/upFile',   //The server endpoint we are connecting to
        success: function (data) {
			//console.log(data[0]);
			//let tableRef = document.getElementById("myTable").getElementsByTagName("tbody")[0];
			//tableRef.innerHTML = "";
			let dummyString = "";
			let i = 0;
			let j = 0;
			let file = "";
			for (i in data){
				var x = document.getElementById("fileSelect");
				var y = document.getElementById("addSelect");
				var z = document.getElementById("desSelect");
				var a = document.getElementById("aSelect");
				var b = document.getElementById("queryFiles");
				var option = document.createElement("option");
				var option2 = document.createElement("option");
				var option3 = document.createElement("option");
				var option4 = document.createElement("option");
				var option5 = document.createElement("option");
				option.text = data[i];
				option2.text = data[i];
				option3.text = data[i];
				option4.text = data[i];
				option5.text = data[i];
				x.add(option);
				y.add(option2);
				z.add(option3);
				a.add(option4);
				b.add(option5);
				
				let obj = new Object();
				let newName = "uploads/";
				let name = data[i];
				newName = newName.concat(data[i]);
				if (i == 0){
					file = newName;
				}
				obj.fileName = newName;
				$.ajax({
					type: 'get',            //Request type
					dataType: 'json',       //Data type - we will use JSON for almost everything 
					data: obj,
					url: '/upDateTable',   //The server endpoint we are connecting to
					success: function (data) {
						
						dummyString = dummyString.concat("<tr>");
						dummyString = dummyString.concat("<td><a href='/uploads/");
						
						dummyString = dummyString.concat(name);
						dummyString = dummyString.concat("'>");
						dummyString = dummyString.concat(name);
						dummyString = dummyString.concat("</td>");
						
						//console.log(data);
						dummyString = dummyString.concat("<td>");
						dummyString = dummyString.concat(data.source);
						dummyString = dummyString.concat("</td>");
						
						dummyString = dummyString.concat("<td>");
						dummyString = dummyString.concat(data.gedcVersion);
						dummyString = dummyString.concat("</td>");
						
						dummyString = dummyString.concat("<td>");
						dummyString = dummyString.concat(data.encoding);
						dummyString = dummyString.concat("</td>");
						
						dummyString = dummyString.concat("<td>");
						dummyString = dummyString.concat(data.subName);
						dummyString = dummyString.concat("</td>");
						
						dummyString = dummyString.concat("<td>");
						dummyString = dummyString.concat(data.subAddress);
						dummyString = dummyString.concat("</td>");
						
						dummyString = dummyString.concat("<td>");
						dummyString = dummyString.concat(data.numIndivs);
						dummyString = dummyString.concat("</td>");
						
						dummyString = dummyString.concat("<td>");
						dummyString = dummyString.concat(data.numFamilies);
						dummyString = dummyString.concat("</td>");
						
						dummyString = dummyString.concat("</a></td>");
						dummyString = dummyString.concat("</tr>");
						
						$('#fileTable').html(dummyString);
						//console.log(dummyString);
					},
					fail: function(error) {
						// Non-200 return, do something with error
						//console.log("hey"); 
					}
				});
				j ++;
			}
			if (j != 0){
				console.log("what");
				document.getElementById("storeButt").disabled = false;
			}
			else{
				console.log("Que?");
				document.getElementById("storeButt").disabled = true;
			}
			let obj2 = {};
			obj2.fileName = file;
			$.ajax({
				type: 'get',            //Request type
				//dataType: 'json',       //Data type - we will use JSON for almost everything 
				data: obj2,
				url: '/upDateFileTable',   //The server endpoint we are connecting to
				success: function (data) {
					console.log(data);
					let json = JSON.parse(data);
					let i = 0;
					let dummyString = "";
					if (json != "[]"){
						for (i in json){
							console.log(json[i]);
							
							let x = document.getElementById("aChoices");
							let y = document.getElementById("desChoices");
							
							let option = document.createElement("option");
							let option2 = document.createElement("option");
							
							option.text = json[i].givenName + "|" + json[i].surname;
							console.log(option.text);
							option2.text = json[i].givenName + "|" + json[i].surname;
							
							x.add(option);
							y.add(option2);
							
							
							dummyString = dummyString.concat("<tr>");
							dummyString = dummyString.concat("<td>");
							dummyString = dummyString.concat(json[i].givenName);
							dummyString = dummyString.concat("</td>");
							
							dummyString = dummyString.concat("<td>");
							dummyString = dummyString.concat(json[i].surname);
							dummyString = dummyString.concat("</td>");
							
							dummyString = dummyString.concat("<td>");
							dummyString = dummyString.concat(json[i].SEX);
							dummyString = dummyString.concat("</td>");
							
							dummyString = dummyString.concat("<td>");
							dummyString = dummyString.concat(json[i].fSize);
							dummyString = dummyString.concat("</td>");
							
							dummyString = dummyString.concat("</tr>");
							
							$('#viewTable').html(dummyString);
						}
					}
				},
				fail: function(error) {
					// Non-200 return, do something with error
					console.log("heyhowareyadoin123123123"); 
				}
			});
        },
        fail: function(error) {
            // Non-200 return, do something with error
            //console.log(error); 
        }
    });
    //let z = y.options[y.selectedIndex].value;
    //let x = y.options[y.selectedIndex].text;
	isDBEmpty();
}

function changeGedTable(){
	let x = document.getElementById('fileSelect');
	let c = document.getElementById("viewTable");
	c.innerHTML = "";
	let obj2 = {};
	let newStr = "uploads/"
	newStr = newStr.concat(x.value);
	console.log(newStr);
	obj2.fileName = newStr;
	
	$.ajax({
		type: 'get',            //Request type
		//dataType: 'json',       //Data type - we will use JSON for almost everything 
		data: obj2,
		url: '/upDateFileTable',   //The server endpoint we are connecting to
		success: function (data) {
			console.log(data);
			let json = JSON.parse(data);
			let i = 0;
			let dummyString = "";
			for (i in json){
				console.log(json[i]);
				
				dummyString = dummyString.concat("<tr>");
				dummyString = dummyString.concat("<td>");
				dummyString = dummyString.concat(json[i].givenName);
				dummyString = dummyString.concat("</td>");
						
				dummyString = dummyString.concat("<td>");
				dummyString = dummyString.concat(json[i].surname);
				dummyString = dummyString.concat("</td>");
						
				dummyString = dummyString.concat("<td>");
				dummyString = dummyString.concat(json[i].SEX);
				dummyString = dummyString.concat("</td>");
						
				dummyString = dummyString.concat("<td>");
				dummyString = dummyString.concat(json[i].fSize);
				dummyString = dummyString.concat("</td>");
						
				dummyString = dummyString.concat("</tr>");
					
				$('#viewTable').html(dummyString);
			}
		},
		fail: function(error) {
			// Non-200 return, do something with error
			console.log("heyhowareyadoin123123123"); 
		}
	});
}


function changeDesChoices(){
	let x = document.getElementById('desSelect');
	let y = document.getElementById("desChoices");
	y.innerHTML = "";
	let obj2 = {};
	let newStr = "uploads/"
	newStr = newStr.concat(x.value);
	console.log(newStr);
	obj2.fileName = newStr;
	$.ajax({
		type: 'get',            //Request type
		//dataType: 'json',       //Data type - we will use JSON for almost everything 
		data: obj2,
		url: '/upDateFileTable',   //The server endpoint we are connecting to
		success: function (data) {
			console.log(data);
			let json = JSON.parse(data);
			let i = 0;
			let dummyString = "";
			for (i in json){
				//let x = document.getElementById("aChoices");
				let y = document.getElementById("desChoices");
						
				//let option = document.createElement("option");
				let option2 = document.createElement("option");
						
				//option.text = json[i].givenName + " " + json[i].surname;
				//console.log(option.text);
				option2.text = json[i].givenName + "|" + json[i].surname;
					
				//x.add(option);
				y.add(option2);
						
			}
		},
		fail: function(error) {
			// Non-200 return, do something with error
			//console.log("heyhowareyadoin123123123"); 
		}
	});
}

function changeAnsChoices(){
	let x = document.getElementById('aSelect');
	let e = document.getElementById("aChoices");
	e.innerHTML = "";
	let obj2 = {};
	let newStr = "uploads/"
	newStr = newStr.concat(x.value);
	console.log(newStr);
	obj2.fileName = newStr;
	$.ajax({
		type: 'get',            //Request type
		//dataType: 'json',       //Data type - we will use JSON for almost everything 
		data: obj2,
		url: '/upDateFileTable',   //The server endpoint we are connecting to
		success: function (data) {
			console.log(data);
			let json = JSON.parse(data);
			let i = 0;
			let dummyString = "";
			for (i in json){
				let x = document.getElementById("aChoices");
				//let y = document.getElementById("desChoices");
						
				let option = document.createElement("option");
				//let option2 = document.createElement("option");
						
				option.text = json[i].givenName + "|" + json[i].surname;
				//console.log(option.text);
				//option2.text = json[i].givenName + " " + json[i].surname;
					
				x.add(option);
				//y.add(option2);
						
			}
		},
		fail: function(error) {
			// Non-200 return, do something with error
			//console.log("heyhowareyadoin123123123"); 
		}
	});
}
function load(){
	loadFiles();
	changeGedTable();
	changeAnsChoices();
}
function isEven(n) {
   return n % 2 == 0;
}

function isOdd(n) {
   return Math.abs(n % 2) == 1;
}
function isDBEmpty(){
	$.ajax({
		type: 'get',
		url: '/numFileTable',   //The server endpoint we are connecting to
		success: function (data) {
			let i = 0;
			for (i in data){}
			let num1 = data[i];
			$.ajax({
				type: 'get',
				url: '/numIndTable',   //The server endpoint we are connecting to
				success: function (data) {
					let i = 0;
					for (i in data){}
					let num2 = data[i];
					if (num1 == 0){
						if (num2 == 0){
							document.getElementById("deleteButt").disabled = true;
							console.log("What no");
						}
						else{
							console.log("hi");
							document.getElementById("deleteButt").disabled = false;
						}
					}
					else{
						console.log("it should be here");
						document.getElementById("deleteButt").disabled = false;
					}
				},
				error: function(data) { 
				}
			});
			
		},
		error: function(data) { 
		}
	});
	console.log("this is dumb");
}

function toTableQuery(data){
	let i = 0;
	let j = 0;
	let dummyString = ""
	for (i in data){
		dummyString = dummyString.concat("<tr>");
		for (j in data[i]){
			dummyString = dummyString.concat("<td>");
			dummyString = dummyString.concat(data[i][j]);
			dummyString = dummyString.concat("</td>");	
		}
		dummyString = dummyString.concat("</tr>");
		//$('#fileTable').html(dummyString);
	}
	document.getElementById("displayQuery").innerHTML = dummyString;
}
