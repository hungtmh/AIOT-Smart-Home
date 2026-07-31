import { useRealtime } from "../realtime/RealtimeContext";

export default function FireAlert() {
  const { fireAlert, clearFireAlert } = useRealtime();

  if (!fireAlert) return null;

  return (
    <div className="fire-alert">
      <h2>🔥 FIRE ALERT</h2>

      <p>Camera: {fireAlert.deviceId}</p>

      <p>Confidence: {(fireAlert.confidence * 100).toFixed(1)}%</p>

      <p>Status: {fireAlert.status}</p>

      <p>
        Time:{" "}
        {fireAlert.detectedAt
          ? new Date(fireAlert.detectedAt).toLocaleString()
          : "Unknown"}
      </p>

      <img
        src={`${import.meta.env.VITE_API_URL}/uploads/${fireAlert.imagePath}`}
        alt="Fire Detection"
      />

      <button onClick={clearFireAlert}>Close</button>
    </div>
  );
}
