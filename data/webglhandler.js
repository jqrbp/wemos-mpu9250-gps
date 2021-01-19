var scene = new THREE.Scene();
var object;
var nquat = new THREE.Quaternion();
var currquat = new THREE.Quaternion();

const AXIS_LENGTH = 430,
    TRACE_SEGMENTS = 15,
    OBJECT_SCALE = 400;

var camera = new THREE.PerspectiveCamera( 75, window.innerWidth/window.innerHeight, 0.1, 1000 );
camera.position.z = 100;

var renderer = new THREE.WebGLRenderer();
renderer.setSize( window.innerWidth, window.innerHeight );
document.body.appendChild( renderer.domElement );

var controls = new THREE.OrbitControls(camera, renderer.domElement);
controls.enableDamping = true;
controls.dampingFactor = 0.25;
controls.enableZoom = true;

var keyLight = new THREE.DirectionalLight(new THREE.Color('hsl(30, 100%, 75%)'), 1.0);
keyLight.position.set(-100, 0, 100);

var fillLight = new THREE.DirectionalLight(new THREE.Color('hsl(240, 100%, 75%)'), 0.75);
fillLight.position.set(100, 0, 100);

var backLight = new THREE.DirectionalLight(0xffffff, 1.0);
backLight.position.set(-100, 0, -100).normalize();

var hemlight = new THREE.HemisphereLight( 0xffffbb, 0x080820, 1 );

scene.add(keyLight);
scene.add(fillLight);
scene.add(backLight);
scene.add(hemlight);

var phongMaterial = new THREE.MeshPhongMaterial({color: 0xffffff});
var mtlLoader = new THREE.MTLLoader();
mtlLoader.setPath('/');
mtlLoader.setMaterialOptions({
   side: THREE.DoubleSide
});
mtlLoader.load('MQ-9-mtl.js', function (materials) {
    materials.preload();
    var objLoader = new THREE.OBJLoader();
    objLoader.setMaterials(materials, phongMaterial);
    objLoader.setPath('/');
    objLoader.load('MQ-9-obj.js', function (obj) {
        object = obj;
        object.scale.x = object.scale.y = object.scale.z = OBJECT_SCALE;
        scene.add(object);
    });
});

function getRotation() {
   if(object != undefined) {
      currquat.w = document.getElementById("qw").value; 
      currquat.x = document.getElementById("qx").value;
      currquat.y = document.getElementById("qy").value;
      currquat.z = document.getElementById("qz").value;
      currquat.normalize();
      nquat = nquat.slerp(currquat, 1 / TRACE_SEGMENTS);
      object.quaternion.w = nquat.w; 
      object.quaternion.x = nquat.x; 
      object.quaternion.y = nquat.y; 
      object.quaternion.z = nquat.z; 
   }
}

function initAxes() {
   var xAxisMat = new THREE.LineBasicMaterial({color: 0xff0000, linewidth: 2});
   var xAxisGeom = new THREE.Geometry();
   xAxisGeom.vertices.push(new THREE.Vector3(0, 0, 0));
   xAxisGeom.vertices.push(new THREE.Vector3(AXIS_LENGTH, 0, 0));
   var xAxis = new THREE.Line(xAxisGeom, xAxisMat);
   scene.add(xAxis);

   var yAxisMat = new THREE.LineBasicMaterial({color: 0x00cc00, linewidth: 2});
   var yAxisGeom = new THREE.Geometry();
   yAxisGeom.vertices.push(new THREE.Vector3(0, 0, 0));
   yAxisGeom.vertices.push(new THREE.Vector3(0, AXIS_LENGTH, 0));
   var yAxis = new THREE.Line(yAxisGeom, yAxisMat);
   scene.add(yAxis);

   var zAxisMat = new THREE.LineBasicMaterial({color: 0x0000ff, linewidth: 2});
   var zAxisGeom = new THREE.Geometry();
   zAxisGeom.vertices.push(new THREE.Vector3(0, 0, 0));
   zAxisGeom.vertices.push(new THREE.Vector3(0, 0, AXIS_LENGTH));
   var zAxis = new THREE.Line(zAxisGeom, zAxisMat);
   scene.add(zAxis);
}

var animate = function () {
   getRotation();
	requestAnimationFrame( animate );
	controls.update();
	renderer.render(scene, camera);
};
initAxes();
animate();