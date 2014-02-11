using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using HFT;

namespace HugeFileTransfer {
    public partial class Form1 : Form {
        private HFTClientWrapper client;
        // private const String serverIp = "212.85.36.40";
        // private const String serverIp = "192.168.0.7";
        private const String serverIp = "127.0.0.1";
        private const String serverPort = "8888";
        private DateTime validUntil = new DateTime(2014, 2, 15);

        public Form1() {
            InitializeComponent();
            iniResults();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e) {
            if (bgTransfer.IsBusy) {
                if (MessageBox.Show("¡Atención! La subida del archivo '" + dlgOpenFile.SafeFileName + "' aún no ha concluido.\n" +
                    "Si cierra el programa ahora, tendrá que terminar la subida más adelante.\n" +
                    "¿Está seguro de que desea cerrar?",
                    "¿Está seguro?", MessageBoxButtons.YesNo, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2) == System.Windows.Forms.DialogResult.No) {
                        e.Cancel = true;
                }
            }
        }

        private void formatResults(Label lbl, object value) {
            if (lbl.Equals(lblResKBps)) {
                double KBps = (double)value;
                if (KBps < 0) {
                    lblResKBps.Text = "N/A";
                }else if (KBps < 950) {
                    lblResKBps.Text = String.Format("{0:#,#0.0#} KB/s", KBps);
                } else {
                    lblResKBps.Text = String.Format("{0:#,#0.0#} MB/s", KBps / 1000);
                }
            } else if (lbl.Equals(lblResRemaining)) {
                double remaining = (double)value;
                string temp = "";

                if (remaining < 0) {
                    lblResRemaining.Text = "N/A";
                } else if (double.IsPositiveInfinity(remaining) || double.IsNegativeInfinity(remaining)) {
                    lblResRemaining.Text = "Infinito";
                } else {
                    temp = String.Format("{0:0}s", remaining % 60);
                    remaining = Math.Floor(remaining / 60);
                    if (remaining > 0) temp = String.Format("{0:0}m", remaining % 60) + temp;
                    remaining = Math.Floor(remaining / 60);
                    if (remaining > 0) temp = String.Format("{0:0}h", remaining % 24) + temp;
                    remaining = Math.Floor(remaining / 24);
                    if (remaining > 0) temp = String.Format("{0:0}d", remaining % 365) + temp;
                    remaining = Math.Floor(remaining / 365);
                    if (remaining > 0) temp = String.Format("{0:0}a", remaining) + temp;
                    lblResRemaining.Text = temp;
                }
            } else if (lbl.Equals(lblResCurrentSize)) {
                Int64 currentSize = (Int64)value;
                if (currentSize < 0) {
                    lblResCurrentSize.Text = "N/A";
                }else if (currentSize < 1000000) {
                    lblResCurrentSize.Text = String.Format("{0:0.00} KB", currentSize / 1000.0);
                } else if (currentSize < 1000000000) {
                    lblResCurrentSize.Text = String.Format("{0:0.00} MB", currentSize / 1000000.0);
                } else {
                    lblResCurrentSize.Text = String.Format("{0:#,#0.00} GB", currentSize / 1000000000.0);
                }
            } else if (lbl.Equals(lblResProgress)) {
                double progress = (double)value;
                if (double.IsNaN(progress) || progress < 0) {
                    progBarUpload.Value = 0;
                    lblResProgress.Text = "N/A";
                } else {
                    progress = Math.Max(0, Math.Min(100, progress));
                    progBarUpload.Value = (int)Math.Round(progress);
                    lblResProgress.Text = String.Format("{0:0.00} %", progress);
                }
            }
        }

        private void iniResults() {
            formatResults(lblResKBps, -1.0);
            formatResults(lblResRemaining, -1.0);
            formatResults(lblResCurrentSize, (Int64)(-1));
            formatResults(lblResProgress, -1.0);
            foreach (Control ctrl in grpResult.Controls) {
                if (!ctrl.Equals(lstResult)) ctrl.Enabled = false;
            }
        }

        private void btnExaminar_Click(object sender, EventArgs e) {
            btnExaminar.Enabled = false;

            dlgOpenFile.FileName = txtClientFileName.Text;
            if (dlgOpenFile.ShowDialog() == System.Windows.Forms.DialogResult.OK) {
                txtClientFileName.Text = dlgOpenFile.FileName;
            }

            btnExaminar.Enabled = true;
        }

        private void btnUpload_Click(object sender, EventArgs e) {
            if (!bgTransfer.IsBusy) {
                btnUpload.Enabled = false;
                foreach (Control ctrl in grpResult.Controls) {
                    ctrl.Enabled = true;
                }
                bgTransfer.RunWorkerAsync();
                if (!bgMonitor.IsBusy) {
                    bgMonitor.RunWorkerAsync();
                }
            }
        }

        private void bgTransfer_DoWork(object sender, DoWorkEventArgs e) {
            BackgroundWorker worker = sender as BackgroundWorker;

            if (worker.CancellationPending == true) {
                e.Cancel = true;
            } else {
                client = new HFTClientWrapper(true, serverIp, serverPort, txtClientFileName.Text, txtServerFileName.Text, 0, "");
                bgTransfer.ReportProgress(100, client.run());
            }
        }

        private void bgTransfer_ProgressChanged(object sender, ProgressChangedEventArgs e) {
            int result = (int)e.UserState;
            iniResults();

            lstResult.Items.AddRange(client.getMessagesList().ToArray());
            if (result >= 0) {
                MessageBox.Show("¡Enhorabuena! El archivo '" + dlgOpenFile.SafeFileName + "' se ha subido correctamente!",
                    "¡Enhorabuena!", MessageBoxButtons.OK, MessageBoxIcon.Information, MessageBoxDefaultButton.Button1);
            } else {
                MessageBox.Show(lstResult.Items[lstResult.Items.Count-1].ToString(),
                    "¡Error!", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
            }
            client.Dispose();
            client = null;
            btnUpload.Enabled = true;
            foreach (Control ctrl in grpResult.Controls) {
                if (!ctrl.Equals(lstResult)) ctrl.Enabled = false;
            }
        }

        private void bgMonitor_DoWork(object sender, DoWorkEventArgs e) {
            BackgroundWorker worker = sender as BackgroundWorker;
            int i = 0;

            if (worker.CancellationPending == true) {
                e.Cancel = true;
            } else {
                while (client == null) {
                    System.Threading.Thread.Sleep(1000);
                }
                client.obtainMonitorResults();  // Asi, aunque calcule mal la velocidad esta primera vez, no muestro los resultados

                while (bgTransfer.IsBusy) {
                    worker.ReportProgress(i);
                    i++;
                    System.Threading.Thread.Sleep(1000);
                }
            }
        }

        private void bgMonitor_ProgressChanged(object sender, ProgressChangedEventArgs e) {
            double KBps, remaining, progress;
            Int64 currentSize;

            if (client == null || !client.obtainMonitorResults()) {
                iniResults();
                return;
            }

            KBps = client.getKBps();
            remaining = client.getRemaining();
            currentSize = client.getCurrentSize();
            progress = client.getCompleted();

            formatResults(lblResKBps, KBps);
            formatResults(lblResRemaining, remaining);
            formatResults(lblResCurrentSize, currentSize);
            formatResults(lblResProgress, progress);

            lstResult.Items.AddRange(client.getMessagesList().ToArray());
        }

        private void Form1_Load(object sender, EventArgs e) {
            if (validUntil.CompareTo(DateTime.UtcNow) <= 0) {
                MessageBox.Show("Lo sentimos, la versión de prueba caducó el " + validUntil.ToString("dd/MM/yyyy") + ".",
                    "¡Error!", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                Close();
            }
        }
    }
}
