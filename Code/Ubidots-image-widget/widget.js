var TOKEN = '';
var DEVICE = 'ESP-test';
var VARIABLE = 'image';
var UPDATE_RATE = 5; // Seconds between image refresh

var imgbox = $('.imgbox');

function getDataFromVariable(device, variable, token, callback) {
  var url = 'https://industrial.api.ubidots.com/api/v1.6/devices/' + device + '/' + variable + '/values/?page_size=1';
  var headers = {
    'X-Auth-Token': token,
    'Content-Type': 'application/json'
  };
  
  $.ajax({
    url: url,
    method: 'GET',
    headers: headers,
    success: function (res) {
      callback(res.results);
    }
  });
};

function fetchImage(){
    getDataFromVariable(DEVICE, VARIABLE, TOKEN, function (values) {
        var b64_img = values[0].context.image_b64;
      imgbox.attr("src", "data:image/png;base64, " + b64_img);
    });
};

setInterval(function(){
    fetchImage()
}, UPDATE_RATE * 1000)

fetchImage();