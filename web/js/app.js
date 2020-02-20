function example_request_json() {
    console.log("Some");
    var fileName = "./data.json";
    $.ajax({
        url: fileName,
        data: {
            "say": "Hello",
            "ask": "time"
        }
    }).done(function(data) {
        console.log(data);
        $('#result').append("<div>" + data.say + "</div>");
    }).fail(function(err){
        console.error(err);
        $('#result').html("error(" + fileName + "): " + err.statusText);
    })
}