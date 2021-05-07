const mtlpath = '/assets/';
const mtlname = '1.mtl';
const objname = '1.obj';
const objscale = 0.002;

var nquat = new THREE.Quaternion();
var currquat = new THREE.Quaternion();
var oquat = new THREE.Quaternion();
var scene = new THREE.Scene();
var object;

const AXIS_LENGTH = 430,
   TRACE_SEGMENTS = 15,
   OBJECT_SCALE = objscale;

function getRotation() {
   if(object != undefined) {
      currquat.w = document.getElementById("qw").value; 
      currquat.x = document.getElementById("qy").value * -1;
      currquat.y = document.getElementById("qz").value * -1;
      currquat.z = document.getElementById("qx").value;
      currquat.normalize();
      oquat.w = document.getElementById("qrw").value; 
      oquat.x = document.getElementById("qrx").value;
      oquat.y = document.getElementById("qry").value;
      oquat.z = document.getElementById("qrz").value;
      oquat.normalize();
      currquat.multiply(oquat);
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

var geometry = new THREE.BufferGeometry();
var positions = [];
var colors = [];
var color = new THREE.Color();
var points;

function addpoints(_x, _y, _z, _max) {
   positions.push( _x, _y, _z );
   // colors
   var vx = ( _x / _max ) + 0.5;
   var vy = ( _y / _max ) + 0.5;
   var vz = ( _z / _max ) + 0.5;
   color.setRGB( vx, vy, vz );
   colors.push( color.r, color.g, color.b );

   geometry.setAttribute( 'position', new THREE.Float32BufferAttribute( positions, 3 ) );
   geometry.setAttribute( 'color', new THREE.Float32BufferAttribute( colors, 3 ) );
   geometry.computeBoundingSphere();

   var material = new THREE.PointsMaterial( { size: 15, vertexColors: THREE.VertexColors } );
   points = new THREE.Points( geometry, material );
   scene.add( points );
}

function webgl_start() { 
   var canvas = document.getElementById("c");

   scene.fog = new THREE.Fog( 0xcce0ff, 10, 3500 );

   // var camera = new THREE.PerspectiveCamera( 75, window.innerWidth/window.innerHeight, 0.1, 1000 );
   // camera.position.z = 100;

   var camera = new THREE.PerspectiveCamera( 30, window.innerWidth/window.innerHeight, 1, 5000 );
   camera.position.set(1000, 300, 200);

   var renderer = new THREE.WebGLRenderer({ canvas: canvas, antialias: true });
   renderer.setSize( window.innerWidth, window.innerHeight );
   document.getElementById("container").appendChild( renderer.domElement );

   var controls = new THREE.OrbitControls(camera, renderer.domElement);
   controls.enableDamping = true;
   controls.dampingFactor = 0.25;
   controls.enableZoom = true;

   // LIGHTS
   var keyLight = new THREE.DirectionalLight(new THREE.Color('hsl(30, 100%, 75%)'), 1.0);
   keyLight.position.set(-100, 0, 100);

   var fillLight = new THREE.DirectionalLight(new THREE.Color('hsl(240, 100%, 75%)'), 0.75);
   fillLight.position.set(100, 0, 100);

   var backLight = new THREE.DirectionalLight(0xffffff, 1.0);
   backLight.position.set(-100, 0, -100).normalize();

   var hemlight = new THREE.HemisphereLight( 0xffffbb, 0x080820, 1 );

   var dirLight = new THREE.DirectionalLight( 0xffffff, 0.5);
   dirLight.color.setHSL( 0.1, 1, 0.95 );
   dirLight.position.set( 2, 50, 2 );
   dirLight.position.multiplyScalar( 30 );
   scene.add( dirLight );
   dirLight.castShadow = true;
   dirLight.shadow.mapSize.width = 2048;
   dirLight.shadow.mapSize.height = 2048;
   var d = 200;
   dirLight.shadow.camera.left = - d;
   dirLight.shadow.camera.right = d;
   dirLight.shadow.camera.top = d;
   dirLight.shadow.camera.bottom = - d;
   dirLight.shadow.camera.far = 2000;
   dirLight.shadow.bias = - 0.0001;

   scene.add(keyLight);
   scene.add(fillLight);
   scene.add(backLight);
   // scene.add(hemlight);
   // scene.add(dirLight);

   // GROUND
   var groundGeo = new THREE.PlaneBufferGeometry( 10000, 10000 );
   var groundMat = new THREE.MeshLambertMaterial( { color: 0xffffff } );
   groundMat.color.setHSL( 0.095, 1, 0.75 );
   var ground = new THREE.Mesh( groundGeo, groundMat );
   ground.position.y = 0;
   ground.rotation.x = - Math.PI / 2;
   ground.castShadow = false;
   ground.receiveShadow = true;
   scene.add( ground );

   //SKY
   var skyGeo = new THREE.SphereGeometry(3000, 32, 15); 
   var material = new THREE.MeshLambertMaterial({ 
      color: 0x7ec0ee,
   });
   var sky = new THREE.Mesh(skyGeo, material);
   sky.material.side = THREE.BackSide;
   scene.add(sky);

   // // Model Car
   var mtlLoader = new THREE.MTLLoader();
   mtlLoader.setPath(mtlpath);
   mtlLoader.setMaterialOptions({
      side: THREE.DoubleSide
   });

   mtlLoader.load(mtlname, function (materials) {
      document.getElementById("loaderTxt").innerHTML = "Loading materials...";
      materials.preload();
      var objLoader = new THREE.OBJLoader();
      objLoader.setMaterials(materials);
      objLoader.setPath(mtlpath);
      objLoader.setResourcePath(mtlpath);
      objLoader.load(objname, function (obj) {
         document.getElementById("loaderTxt").innerHTML = "Loading object mesh...";
         object = obj;
         object.scale.x = object.scale.y = object.scale.z = OBJECT_SCALE;
         object.castShadow = true;
         object.receiveShadow = true;
         object.traverse( function ( child ) {
                  if ( child instanceof THREE.Mesh ) {
                     child.castShadow = true;
                     child.receiveShadow = true;
                  }
            } );
         scene.add(object);
         document.getElementById("loader").style.visibility = 'hidden';
      })
   });

   var animate = function () {
      getRotation();
      requestAnimationFrame( animate );
      controls.update();
      renderer.render(scene, camera);
   };
   initAxes();
   animate();
}