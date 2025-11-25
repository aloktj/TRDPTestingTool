import { useMemo, useState } from 'react'
import './App.css'
import ConfigurationsView from './components/ConfigurationsView'
import PdView from './components/PdView'

type Page = 'pd' | 'configs'

function App() {
  const [activePage, setActivePage] = useState<Page>('pd')

  const header = useMemo(() => {
    if (activePage === 'configs') {
      return {
        eyebrow: 'Configuration management',
        title: 'Configurations',
        subtitle:
          'Manage TRDP XML configuration files, inspect their summaries, and activate them for the engine.',
        status: 'Ready',
      }
    }

    return {
      eyebrow: 'Runtime explorer',
      title: 'Process Data telegrams',
      subtitle: 'Monitor TRDP PD telegrams and pick one to inspect its latest activity.',
      status: 'Connected',
    }
  }, [activePage])

  const renderPage = activePage === 'configs' ? <ConfigurationsView /> : <PdView />

  return (
    <div className="app-shell">
      <aside className="sidebar">
        <div className="sidebar__brand">TRDP Testing Tool</div>
        <nav className="sidebar__nav">
          <button
            className={`nav-link ${activePage === 'pd' ? 'nav-link--active' : ''}`}
            type="button"
            onClick={() => setActivePage('pd')}
          >
            PD telegrams
          </button>
          <button
            className={`nav-link ${activePage === 'configs' ? 'nav-link--active' : ''}`}
            type="button"
            onClick={() => setActivePage('configs')}
          >
            Configurations
          </button>
          <button className="nav-link" type="button" disabled>
            Reports
          </button>
          <button className="nav-link" type="button" disabled>
            Settings
          </button>
        </nav>
      </aside>
      <main className="main">
        <header className="main__header">
          <div>
            <p className="eyebrow">{header.eyebrow}</p>
            <h1>{header.title}</h1>
            <p className="subtitle">{header.subtitle}</p>
          </div>
          <div className="status-pill">{header.status}</div>
        </header>
        <section className="main__content main__content--full">{renderPage}</section>
      </main>
    </div>
  )
}

export default App
