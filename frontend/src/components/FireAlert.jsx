import { useRealtime } from "../realtime/RealtimeContext";

export default function FireAlert() {
  const { fireAlert, clearFireAlert } = useRealtime();

  if (!fireAlert) return null;

  return (
    <div
      style={{
        position: "fixed",
        inset: 0,
        background: "rgba(0,0,0,0.45)",
        display: "flex",
        justifyContent: "center",
        alignItems: "center",
        zIndex: 9999,
      }}
    >
      <div
        style={{
          position: "relative",
          width: "520px",
          maxWidth: "90%",
          background: "#ffffff",
          borderRadius: "18px",
          padding: "28px",
          boxShadow: "0 20px 50px rgba(0,0,0,0.25)",
        }}
      >
        {/* Nút đóng */}
        <button
          onClick={clearFireAlert}
          style={{
            position: "absolute",
            top: "14px",
            right: "14px",
            width: "34px",
            height: "34px",
            border: "none",
            borderRadius: "50%",
            background: "#f3f4f6",
            cursor: "pointer",
            fontSize: "20px",
            fontWeight: "bold",
            color: "#666",
          }}
        >
          ×
        </button>

        <h2
          style={{
            margin: 0,
            marginBottom: "20px",
            color: "#dc2626",
            fontSize: "28px",
            fontWeight: "700",
            textAlign: "center",
          }}
        >
          Fire Alert
        </h2>

        <img
          src={`${import.meta.env.VITE_API_URL}/uploads/${fireAlert.imagePath}`}
          alt="Fire Detection"
          style={{
            width: "100%",
            borderRadius: "12px",
            border: "1px solid #e5e7eb",
            marginBottom: "20px",
          }}
        />

        <div
          style={{
            background: "#f9fafb",
            borderRadius: "12px",
            padding: "16px",
            lineHeight: "2",
            color: "#374151",
          }}
        >
          <div>
            <strong>Camera:</strong> {fireAlert.deviceId}
          </div>

          <div>
            <strong>Confidence:</strong>{" "}
            {(fireAlert.confidence * 100).toFixed(1)}%
          </div>

          <div>
            <strong>Status:</strong> {fireAlert.status}
          </div>

          <div>
            <strong>Detected At:</strong>{" "}
            {fireAlert.detectedAt
              ? new Date(fireAlert.detectedAt).toLocaleString()
              : "Unknown"}
          </div>
        </div>

        <button
          onClick={clearFireAlert}
          style={{
            marginTop: "22px",
            width: "100%",
            padding: "12px",
            border: "none",
            borderRadius: "10px",
            background: "#2563eb",
            color: "#fff",
            fontSize: "15px",
            fontWeight: "600",
            cursor: "pointer",
          }}
        >
          Close
        </button>
      </div>
    </div>
  );
}
