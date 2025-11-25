class EngineController {
  constructor() {
    this.activeConfig = null;
    this.running = false;
  }

  async loadConfig(xmlContent) {
    this.activeConfig = xmlContent;
  }

  async restart() {
    this.running = false;
    await new Promise((resolve) => setTimeout(resolve, 25));
    this.running = true;
  }
}

module.exports = new EngineController();
