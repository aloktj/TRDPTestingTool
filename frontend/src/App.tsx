import './App.css'
import PdView from './components/PdView'

function App() {
  return (
    <div className="app-shell">
      <aside className="sidebar">
        <div className="sidebar__brand">TRDP Testing Tool</div>
        <nav className="sidebar__nav">
          <a className="nav-link" href="#">Overview</a>
          <a className="nav-link" href="#" aria-current="page">
            PD telegrams
          </a>
          <a className="nav-link" href="#">Scenarios</a>
          <a className="nav-link" href="#">Reports</a>
          <a className="nav-link" href="#">Settings</a>
        </nav>
      </aside>
      <main className="main">
        <header className="main__header">
          <div>
            <p className="eyebrow">Runtime explorer</p>
            <h1>Process Data telegrams</h1>
            <p className="subtitle">Monitor TRDP PD telegrams and pick one to inspect its latest activity.</p>
          </div>
          <div className="status-pill">Connected</div>
        </header>
        <section className="main__content main__content--full">
          <PdView />
        </section>
      </main>
    </div>
  )
}

export default App
