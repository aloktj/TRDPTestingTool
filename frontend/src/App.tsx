import './App.css'

function App() {
  return (
    <div className="app-shell">
      <aside className="sidebar">
        <div className="sidebar__brand">TRDP Testing Tool</div>
        <nav className="sidebar__nav">
          <a className="nav-link" href="#">Overview</a>
          <a className="nav-link" href="#">Scenarios</a>
          <a className="nav-link" href="#">Reports</a>
          <a className="nav-link" href="#">Settings</a>
        </nav>
      </aside>
      <main className="main">
        <header className="main__header">
          <div>
            <p className="eyebrow">Welcome</p>
            <h1>Project Dashboard</h1>
            <p className="subtitle">
              Start exploring the TRDP data, manage scenarios, and review results in a unified
              workspace.
            </p>
          </div>
          <div className="status-pill">Connected</div>
        </header>
        <section className="main__content">
          <div className="panel">
            <h2>Next steps</h2>
            <ul>
              <li>Configure your connection to the backend API.</li>
              <li>Load or create a scenario to begin testing.</li>
              <li>Review the latest runs and share reports with your team.</li>
            </ul>
          </div>
          <div className="panel">
            <h2>Backend proxy</h2>
            <p>
              Requests to <code>/api/*</code> are proxied to the Drogon backend on{' '}
              <span className="highlight">localhost:8080</span> when running the dev server.
            </p>
          </div>
        </section>
      </main>
    </div>
  )
}

export default App
