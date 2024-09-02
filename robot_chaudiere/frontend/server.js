const webcam = document.getElementById('webcam');
const startButton = document.getElementById('start');
const stopButton = document.getElementById('stop');
const recordedVideo = document.getElementById('recorded');

let mediaRecorder;
let recordedChunks = [];

async function startRecording() {
  recordedChunks = [];
  const stream = await navigator.mediaDevices.getUserMedia({ video: true });
  webcam.srcObject = stream;

  mediaRecorder = new MediaRecorder(stream);

  mediaRecorder.ondataavailable = event => {
    if (event.data.size > 0) {
      recordedChunks.push(event.data);
    }
  };

  mediaRecorder.onstop = () => {
    const blob = new Blob(recordedChunks, { type: 'video/webm' });
    const url = URL.createObjectURL(blob);
    recordedVideo.src = url;
    recordedVideo.style.display = 'block';
    recordedVideo.controls = true;
  };

  mediaRecorder.start();
  startButton.disabled = true;
  stopButton.disabled = false;
}

function stopRecording() {
  mediaRecorder.stop();
  startButton.disabled = false;
  stopButton.disabled = true;
}

startButton.addEventListener('click', startRecording);
stopButton.addEventListener('click', stopRecording); 
