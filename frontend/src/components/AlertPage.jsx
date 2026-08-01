import { useEffect, useState } from "react";
import { useRealtime } from "../realtime/RealtimeContext";
import { commandDevice } from "../lib/api";
import { icons } from "./icons";

const { Flame, AlertTriangle, ShieldCheck, Power } = icons;

function AlertPage() {
  const { fireAlert: realtimeAlert } = useRealtime();

  const [fireAlert, setFireAlert] = useState(null);

  useEffect(() => {
    async function loadLatestAlert() {
      try {
        const res = await fetch("http://localhost:8080/api/fire-alert/latest");

        if (res.status === 204) {
          setFireAlert(null);
          return;
        }

        if (!res.ok) {
          throw new Error("Failed to load latest fire alert");
        }

        const data = await res.json();
        setFireAlert(data);
      } catch (error) {
        console.error("Failed to load latest fire alert:", error);
      }
    }

    loadLatestAlert();
  }, []);

  useEffect(() => {
    if (realtimeAlert) {
      setFireAlert(realtimeAlert);
    }
  }, [realtimeAlert]);

  async function handleTurnOnPump() {
    try {
      await commandDevice("pump", true);
    } catch (error) {
      console.error("Failed to turn on pump:", error);
    }
  }

  return (
    <>
      <header className="dashboard-header">
        <div>
          <div className="eyebrow">
            <AlertTriangle size={16} aria-hidden="true" />
            System Alerts
          </div>
          <h1>Fire Alert Monitor</h1>
        </div>
      </header>

      <section className="status-layout" aria-label="Fire Alert Status">
        {fireAlert ? (
          <article
            className="panel safety-panel"
            style={{ borderColor: "var(--red)", borderWidth: "2px" }}
          >
            <div className="panel-title">
              <div>
                <Flame size={24} aria-hidden="true" color="var(--red)" />
                <h2 style={{ color: "var(--red)" }}>FIRE DETECTED</h2>
              </div>
              <span
                className="safe-pill"
                style={{ backgroundColor: "var(--red)", color: "white" }}
              >
                DANGER
              </span>
            </div>

            <div
              style={{
                margin: "1rem 0",
                display: "flex",
                flexDirection: "column",
                alignItems: "center",
                gap: "1rem",
              }}
            >
              {fireAlert.imagePath && (
                <img
                  src={`http://localhost:8080/${fireAlert.imagePath}`}
                  alt="Fire Detection"
                  style={{
                    maxWidth: "100%",
                    borderRadius: "8px",
                    border: "1px solid var(--red)",
                  }}
                />
              )}
              <div
                style={{
                  display: "flex",
                  flexDirection: "column",
                  gap: "0.5rem",
                  width: "100%",
                  alignItems: "center",
                }}
              >
                <p style={{ color: "var(--red)", fontWeight: "bold" }}>
                  A fire has been detected at{" "}
                  {new Date(fireAlert.detectedAt).toLocaleString("vi-VN")}
                </p>

                <button
                  className="btn-action"
                  style={{
                    backgroundColor: "var(--red)",
                    color: "white",
                    padding: "1rem 2rem",
                    fontSize: "1.1rem",
                    marginTop: "1rem",
                    display: "flex",
                    alignItems: "center",
                    gap: "0.5rem",
                  }}
                  onClick={handleTurnOnPump}
                >
                  <Power size={20} />
                  Xác nhận bật máy bơm (Turn on Pump)
                </button>
              </div>
            </div>
          </article>
        ) : (
          <article className="panel">
            <div className="panel-title">
              <div>
                <ShieldCheck
                  size={24}
                  aria-hidden="true"
                  color="var(--green)"
                />
                <h2>System is Safe</h2>
              </div>
              <span className="safe-pill">Safe</span>
            </div>
            <div
              style={{
                padding: "2rem",
                textAlign: "center",
                color: "var(--text-muted)",
              }}
            >
              <p>
                Hiện không có cảnh báo cháy nổ. (No fire alerts at the moment.)
              </p>
            </div>
          </article>
        )}
      </section>
    </>
  );
}

export default AlertPage;
